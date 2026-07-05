#include "ui/SettingsDialog.h"
#include "app/Application.h"
#include "app/Settings.h"
#include "network/ImageUploader.h"
#include "utils/ImageCache.h"
#include "utils/ThemeManager.h"
#include "ui/AppleTitleBar.h"
#include "ui/MessageBox.h"
#include "utils/Logger.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QTabWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QSpinBox>
#include <QFontComboBox>
#include <QCheckBox>
#include <QLabel>
#include <QFileDialog>
#include <QMessageBox>
#include <QFont>
#include <QApplication>

SettingsDialog::SettingsDialog(QWidget* parent)
    : QDialog(parent)
    , m_downloadLocationEdit(new QLineEdit(this))
    , m_browseBtn(new QPushButton(tr("浏览..."), this))
    , m_cacheSizeLabel(new QLabel(this))
    , m_clearCacheBtn(new QPushButton(tr("清除缓存"), this))
    , m_themeCombo(new QComboBox(this))
    , m_fontSizeSpin(new QSpinBox(this))
    , m_fontCombo(new QFontComboBox(this))
    , m_storageDestCombo(new QComboBox(this))
    , m_outputFormatCombo(new QComboBox(this))
    , m_cdnDomainCombo(new QComboBox(this))
    , m_applyBtn(new QPushButton(tr("应用"), this))
    , m_resetBtn(new QPushButton(tr("重置"), this))
{
    setObjectName("settingsDialog");
    setupUi();
    loadSettings();
}

void SettingsDialog::setupUi() {
    setWindowTitle(tr("设置"));
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    resize(540, 590);
    setMinimumSize(480, 430);

    auto* outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(0,0,0,0);
    outerLayout->setSpacing(0);

    auto* titleBar = new AppleTitleBar(this, tr("设置"), false);
    outerLayout->addWidget(titleBar);

    auto* mainLayout = new QVBoxLayout();
    mainLayout->setContentsMargins(24, 18, 24, 24);
    mainLayout->setSpacing(16);
    outerLayout->addLayout(mainLayout);

    auto* tabs = new QTabWidget(this);

    // ---- General Tab ----
    auto* generalTab = new QWidget();
    auto* generalLayout = new QFormLayout(generalTab);
    generalLayout->setSpacing(16);
    generalLayout->setContentsMargins(12, 16, 12, 16);

    // Download location
    auto* dlLayout = new QHBoxLayout();
    dlLayout->addWidget(m_downloadLocationEdit, 1);
    dlLayout->addWidget(m_browseBtn);
    auto* dlLabel = new QLabel(tr("下载位置"));
    dlLabel->setProperty("settingsLabel", true);
    generalLayout->addRow(dlLabel, dlLayout);

    // Cache
    auto* cacheLayout = new QHBoxLayout();
    cacheLayout->addWidget(m_cacheSizeLabel, 1);
    cacheLayout->addWidget(m_clearCacheBtn);
    auto* cacheLabel = new QLabel(tr("缓存管理"));
    cacheLabel->setProperty("settingsLabel", true);
    generalLayout->addRow(cacheLabel, cacheLayout);

    generalLayout->addRow("", new QLabel(tr(""))); // spacer

    tabs->addTab(generalTab, tr("通用"));

    // ---- Appearance Tab ----
    auto* appearanceTab = new QWidget();
    auto* appearanceLayout = new QFormLayout(appearanceTab);
    appearanceLayout->setSpacing(16);
    appearanceLayout->setContentsMargins(12, 16, 12, 16);

    m_themeCombo->addItems({
        tr("晨雾青"), tr("玫瑰金"), tr("珊瑚橙"), tr("薰衣草"), tr("奶茶色"), tr("清透蓝"), tr("深色暗夜")
    });
    m_themeCombo->setItemData(0, "mint");
    m_themeCombo->setItemData(1, "rose");
    m_themeCombo->setItemData(2, "sky");
    m_themeCombo->setItemData(3, "lavender");
    m_themeCombo->setItemData(4, "sunset");
    m_themeCombo->setItemData(5, "ocean");
    m_themeCombo->setItemData(6, "dark");
    auto* themeLabel = new QLabel(tr("配色主题"));
    themeLabel->setProperty("settingsLabel", true);
    appearanceLayout->addRow(themeLabel, m_themeCombo);

    // Font size: - button | value | + button
    auto* fontSizeWidget = new QWidget();
    auto* fontSizeHLayout = new QHBoxLayout(fontSizeWidget);
    fontSizeHLayout->setContentsMargins(0,0,0,0);
    fontSizeHLayout->setSpacing(4);

    auto* minusBtn = new QPushButton("−");
    minusBtn->setFixedSize(32, 32);
    minusBtn->setProperty("stepperBtn", true);
    connect(minusBtn, &QPushButton::clicked, this, [this]() {
        int v = m_fontSizeSpin->value();
        if (v > m_fontSizeSpin->minimum()) m_fontSizeSpin->setValue(v - 1);
    });

    auto* plusBtn = new QPushButton("+");
    plusBtn->setFixedSize(32, 32);
    plusBtn->setProperty("stepperBtn", true);
    connect(plusBtn, &QPushButton::clicked, this, [this]() {
        int v = m_fontSizeSpin->value();
        if (v < m_fontSizeSpin->maximum()) m_fontSizeSpin->setValue(v + 1);
    });

    m_fontSizeSpin->setRange(8, 24);
    m_fontSizeSpin->setSuffix(" pt");
    m_fontSizeSpin->setAlignment(Qt::AlignCenter);
    m_fontSizeSpin->setButtonSymbols(QSpinBox::NoButtons);
    m_fontSizeSpin->setFixedWidth(70);

    fontSizeHLayout->addWidget(minusBtn);
    fontSizeHLayout->addWidget(m_fontSizeSpin);
    fontSizeHLayout->addWidget(plusBtn);
    fontSizeHLayout->addStretch();

    auto* fontSizeLabel = new QLabel(tr("字号"));
    fontSizeLabel->setProperty("settingsLabel", true);
    appearanceLayout->addRow(fontSizeLabel, fontSizeWidget);

    auto* fontLabel = new QLabel(tr("字体"));
    fontLabel->setProperty("settingsLabel", true);
    appearanceLayout->addRow(fontLabel, m_fontCombo);

    tabs->addTab(appearanceTab, tr("外观"));

    // ---- Upload Tab ----
    auto* uploadTab = new QWidget();
    auto* uploadLayout = new QFormLayout(uploadTab);
    uploadLayout->setSpacing(16);
    uploadLayout->setContentsMargins(12, 16, 12, 16);

    m_storageDestCombo->addItems({"auto", "local", "telegram", "r2"});
    auto* storageLabel = new QLabel(tr("默认存储"));
    storageLabel->setProperty("settingsLabel", true);
    uploadLayout->addRow(storageLabel, m_storageDestCombo);

    m_outputFormatCombo->addItems({"auto", "jpg", "png", "webp", "gif", "webp_animated"});
    auto* formatLabel = new QLabel(tr("默认格式"));
    formatLabel->setProperty("settingsLabel", true);
    uploadLayout->addRow(formatLabel, m_outputFormatCombo);

    m_cdnDomainCombo->addItems({
        "img.scdn.io", "cloudflareimg.cdn.sn", "edgeoneimg.cdn.sn",
        "esaimg.cdn1.vip", "cloudflarecnimg.scdn.io", "anycastimg.scdn.io", "edgeoneimg.cdn1.vip"
    });
    auto* cdnLabel = new QLabel(tr("默认 CDN"));
    cdnLabel->setProperty("settingsLabel", true);
    uploadLayout->addRow(cdnLabel, m_cdnDomainCombo);

    tabs->addTab(uploadTab, tr("上传"));

    mainLayout->addWidget(tabs);

    // Buttons
    auto* btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    btnLayout->addWidget(m_resetBtn);
    btnLayout->addWidget(m_applyBtn);
    mainLayout->addLayout(btnLayout);

    // Connections
    connect(m_browseBtn, &QPushButton::clicked, this, &SettingsDialog::onBrowseDownloadLocation);
    connect(m_clearCacheBtn, &QPushButton::clicked, this, &SettingsDialog::onClearCache);
    connect(m_applyBtn, &QPushButton::clicked, this, &SettingsDialog::onApply);
    connect(m_resetBtn, &QPushButton::clicked, this, &SettingsDialog::onReset);
    connect(m_themeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &SettingsDialog::onThemeChanged);
    connect(m_fontSizeSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &SettingsDialog::onFontSizeChanged);
    connect(m_fontCombo, &QFontComboBox::currentFontChanged,
            this, &SettingsDialog::onFontFamilyChanged);
}

void SettingsDialog::loadSettings() {
    auto* s = Application::instance()->settings();

    m_downloadLocationEdit->setText(s->downloadLocation());

    auto* cache = Application::instance()->imageCache();
    qint64 cacheSize = cache->totalDiskUsage();
    m_cacheSizeLabel->setText(tr("磁盘: %1 MB | 内存: %2 项")
        .arg(cacheSize / (1024.0 * 1024.0), 0, 'f', 1)
        .arg(200)); // memory cache size

    QString theme = s->theme();
    int themeIdx = m_themeCombo->findData(theme);
    if (themeIdx >= 0) m_themeCombo->setCurrentIndex(themeIdx);

    // Font presets
    QStringList fonts = ThemeManager::fontPresets();
    m_fontCombo->clear();
    for (const auto& key : fonts) {
        m_fontCombo->addItem(ThemeManager::fontPresetDisplayName(key), key);
    }

    m_fontSizeSpin->setValue(s->fontSize());
    m_fontCombo->setCurrentFont(QFont(s->fontFamily()));

    m_storageDestCombo->setCurrentText(s->defaultStorageDest());
    m_outputFormatCombo->setCurrentText(s->defaultOutputFormat());
    m_cdnDomainCombo->setCurrentText(s->defaultCdnDomain());
}

void SettingsDialog::saveSettings() {
    auto* s = Application::instance()->settings();

    s->setDownloadLocation(m_downloadLocationEdit->text());
    s->setTheme(m_themeCombo->currentData().toString());
    s->setFontSize(m_fontSizeSpin->value());
    s->setFontFamily(m_fontCombo->currentFont().family());
    s->setDefaultStorageDest(m_storageDestCombo->currentText());
    s->setDefaultOutputFormat(m_outputFormatCombo->currentText());
    s->setDefaultCdnDomain(m_cdnDomainCombo->currentText());

    s->save();
}

void SettingsDialog::onBrowseDownloadLocation() {
    QString dir = QFileDialog::getExistingDirectory(this, tr("选择下载位置"));
    if (!dir.isEmpty()) {
        m_downloadLocationEdit->setText(dir);
    }
}

void SettingsDialog::onClearCache() {
    auto reply = MessageBox::question(this, tr("清除缓存"),
        tr("确定要清除所有缓存的图片吗？"),
        QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        Application::instance()->imageCache()->clearAll();
        loadSettings();
    }
}

void SettingsDialog::onThemeChanged(int index) {
    QString theme = m_themeCombo->itemData(index).toString();
    Application::instance()->settings()->setTheme(theme);
    Application::instance()->settings()->save();
    // Apply immediately
    ThemeManager::instance().setTheme(theme);
    bool isDark = (theme == "dark");
    qApp->setStyleSheet(ThemeManager::instance().buildStylesheet(isDark));
}
void SettingsDialog::onFontSizeChanged(int) { /* Preview */ }
void SettingsDialog::onFontFamilyChanged(const QFont&) { /* Preview */ }

void SettingsDialog::onApply() {
    saveSettings();
    MessageBox::information(this, tr("设置"), tr("设置已保存。"));
    accept();
}

void SettingsDialog::onReset() {
    loadSettings();
}
