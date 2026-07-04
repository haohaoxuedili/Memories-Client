#include "utils/ImageCache.h"
#include "utils/Logger.h"
#include <QCryptographicHash>
#include <QFileInfo>
#include <QDirIterator>
#include <QStandardPaths>

ImageCache::ImageCache(QObject* parent)
    : QObject(parent)
    , m_memoryCache(200) // 200 pixmaps in memory
{
    QString defaultCache = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    if (defaultCache.isEmpty()) {
        defaultCache = QDir::homePath() + "/.cache/MemoriesClient";
    }
    setCacheDir(defaultCache + "/images");
}

ImageCache::~ImageCache() {
    clearMemory();
}

void ImageCache::setCacheDir(const QString& dir) {
    QMutexLocker lock(&m_mutex);
    m_cacheDir = QDir(dir);
    if (!m_cacheDir.exists()) {
        m_cacheDir.mkpath(".");
    }
    LOG_INFO("ImageCache dir set to: " + dir);
}

QString ImageCache::cacheDir() const {
    return m_cacheDir.absolutePath();
}

void ImageCache::setMaxSizeBytes(qint64 bytes) {
    m_maxDiskBytes = bytes;
    pruneDiskCache(bytes);
}

void ImageCache::setMaxMemoryItems(int count) {
    m_memoryCache.setMaxCost(count);
}

QPixmap ImageCache::getPixmap(const QString& key) {
    QMutexLocker lock(&m_mutex);

    // Try memory cache first
    if (auto* pix = m_memoryCache.object(key)) {
        return *pix;
    }

    // Try disk cache
    QString path = toLocalPath(key);
    if (QFileInfo::exists(path)) {
        QPixmap pix(path);
        if (!pix.isNull()) {
            m_memoryCache.insert(key, new QPixmap(pix));
            return pix;
        }
    }
    return QPixmap();
}

QPixmap ImageCache::getLocalPixmap(const QString& localPath) {
    return getPixmap(localPath);
}

void ImageCache::putPixmap(const QString& key, const QPixmap& pixmap) {
    QMutexLocker lock(&m_mutex);

    // Memory cache
    m_memoryCache.insert(key, new QPixmap(pixmap));

    // Disk cache
    QString path = toLocalPath(key);
    QFileInfo fi(path);
    QDir().mkpath(fi.absolutePath());
    pixmap.save(path, "PNG");

    // Check disk usage periodically
    pruneDiskCache(m_maxDiskBytes);
}

QString ImageCache::localPath(const QString& urlKey) const {
    return toLocalPath(urlKey);
}

bool ImageCache::hasLocal(const QString& urlKey) const {
    QMutexLocker lock(&m_mutex);
    if (m_memoryCache.contains(urlKey)) return true;
    return QFileInfo::exists(toLocalPath(urlKey));
}

void ImageCache::removeLocal(const QString& urlKey) {
    QMutexLocker lock(&m_mutex);
    m_memoryCache.remove(urlKey);
    QFile::remove(toLocalPath(urlKey));
}

void ImageCache::clearMemory() {
    QMutexLocker lock(&m_mutex);
    m_memoryCache.clear();
}

void ImageCache::clearAll() {
    QMutexLocker lock(&m_mutex);
    m_memoryCache.clear();

    // Remove disk cache
    QDirIterator it(m_cacheDir.absolutePath(),
                    QDir::Files | QDir::NoDotAndDotDot,
                    QDirIterator::Subdirectories);
    while (it.hasNext()) {
        QFile::remove(it.next());
    }
    emit cacheCleared();
}

qint64 ImageCache::totalDiskUsage() const {
    qint64 total = 0;
    QDirIterator it(m_cacheDir.absolutePath(),
                    QDir::Files | QDir::NoDotAndDotDot,
                    QDirIterator::Subdirectories);
    while (it.hasNext()) {
        total += QFileInfo(it.next()).size();
    }
    return total;
}

void ImageCache::pruneDiskCache(qint64 maxBytes) {
    // Simple LRU-style pruning: remove oldest files if over limit
    qint64 current = totalDiskUsage();
    if (current <= maxBytes) return;

    QList<QPair<QDateTime, QString>> files;
    QDirIterator it(m_cacheDir.absolutePath(),
                    QDir::Files | QDir::NoDotAndDotDot,
                    QDirIterator::Subdirectories);
    while (it.hasNext()) {
        QString path = it.next();
        QFileInfo fi(path);
        files.append({fi.lastModified(), path});
    }

    std::sort(files.begin(), files.end(),
              [](const auto& a, const auto& b) { return a.first < b.first; });

    for (const auto& [time, path] : files) {
        if (current <= maxBytes * 0.8) break;
        QFileInfo fi(path);
        current -= fi.size();
        QFile::remove(path);
    }
}

QString ImageCache::toLocalPath(const QString& key) const {
    QByteArray hash = QCryptographicHash::hash(
        key.toUtf8(), QCryptographicHash::Sha256).toHex().left(16);
    return m_cacheDir.absoluteFilePath(hash + ".png");
}
