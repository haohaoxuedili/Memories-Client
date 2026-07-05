#include "ui/ImageInfoDialog.h"
#include "utils/Logger.h"
#include "ui/AppleTitleBar.h"

#include <QVBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QTextEdit>
#include <QPushButton>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>
#include <QClipboard>
#include <QApplication>
#include <QScrollArea>
#include <QStyle>

// Helper: format file size
static QString formatFileSize(qint64 bytes) {
    if (bytes < 1024) return QString::number(bytes) + " B";
    if (bytes < 1024 * 1024) return QString::number(bytes / 1024.0, 'f', 1) + " KB";
    if (bytes < 1024LL * 1024 * 1024) return QString::number(bytes / (1024.0 * 1024.0), 'f', 1) + " MB";
    return QString::number(bytes / (1024.0 * 1024.0 * 1024.0), 'f', 2) + " GB";
}

ImageInfoDialog::ImageInfoDialog(QWidget* parent)
    : QDialog(parent)
    , m_nameLabel(new QLabel(this))
    , m_storageLabel(new QLabel(this))
    , m_tagsLabel(new QLabel(this))
    , m_sizeLabel(new QLabel(this))
    , m_dimensionsLabel(new QLabel(this))
    , m_uploadDateLabel(new QLabel(this))
    , m_locationLabel(new QLabel(this))
    , m_urlEdit(new QTextEdit(this))
    , m_descEdit(new QTextEdit(this))
    , m_loadingLabel(new QLabel(tr("加载中..."), this))
    , m_manager(new QNetworkAccessManager(this))
{
    setObjectName("imageInfoDialog");
    setupUi();
}

void ImageInfoDialog::setupUi() {
    setWindowTitle(tr("图片信息"));
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    resize(580, 620);
    setMinimumSize(500, 460);

    auto* outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(0,0,0,0);
    outerLayout->setSpacing(0);

    auto* titleBar = new AppleTitleBar(this, tr("图片信息"), false);
    outerLayout->addWidget(titleBar);

    auto* mainLayout = new QVBoxLayout();
    mainLayout->setContentsMargins(24, 18, 24, 24);
    mainLayout->setSpacing(16);
    outerLayout->addLayout(mainLayout);

    m_loadingLabel->setAlignment(Qt::AlignCenter);
    m_loadingLabel->setProperty("infoLoading", true);
    mainLayout->addWidget(m_loadingLabel);

    // Info group
    auto* infoGroup = new QGroupBox(tr("图片详情"));
    infoGroup->setProperty("infoSection", true);
    auto* infoLayout = new QFormLayout(infoGroup);
    infoLayout->setContentsMargins(16, 24, 16, 16);
    infoLayout->setHorizontalSpacing(14);
    infoLayout->setVerticalSpacing(12);
    infoLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);

    const QList<QLabel*> valueLabels = {
        m_nameLabel, m_storageLabel, m_tagsLabel, m_sizeLabel,
        m_uploadDateLabel, m_locationLabel
    };
    for (auto* label : valueLabels) {
        label->setProperty("infoValue", true);
        label->setTextInteractionFlags(Qt::TextSelectableByMouse);
        label->setWordWrap(true);
    }

    infoLayout->addRow(tr("文件名"), m_nameLabel);
    infoLayout->addRow(tr("存储位置"), m_storageLabel);
    infoLayout->addRow(tr("标签"), m_tagsLabel);
    m_tagsLabel->setWordWrap(true);
    infoLayout->addRow(tr("大小"), m_sizeLabel);
    infoLayout->addRow(tr("上传日期"), m_uploadDateLabel);
    infoLayout->addRow(tr("归属地"), m_locationLabel);
    infoLayout->addRow(tr("链接"), m_urlEdit);
    m_urlEdit->setReadOnly(true);
    m_urlEdit->setAcceptRichText(false);
    m_urlEdit->setLineWrapMode(QTextEdit::WidgetWidth);
    m_urlEdit->setMinimumHeight(74);
    m_urlEdit->setMaximumHeight(104);
    m_urlEdit->setProperty("infoUrlBox", true);

    mainLayout->addWidget(infoGroup);

    // Description group
    auto* descGroup = new QGroupBox(tr("描述"));
    descGroup->setProperty("infoSection", true);
    auto* descLayout = new QVBoxLayout(descGroup);
    descLayout->setContentsMargins(16, 24, 16, 16);
    m_descEdit->setReadOnly(true);
    m_descEdit->setAcceptRichText(false);
    m_descEdit->setMinimumHeight(96);
    m_descEdit->setMaximumHeight(140);
    m_descEdit->setProperty("infoDescBox", true);
    descLayout->addWidget(m_descEdit);
    mainLayout->addWidget(descGroup);

    // Buttons
    auto* btnLayout = new QHBoxLayout();
    auto* copyBtn = new QPushButton(tr("复制链接"), this);
    connect(copyBtn, &QPushButton::clicked, this, [this]() {
        QApplication::clipboard()->setText(m_urlEdit->toPlainText());
    });
    btnLayout->addWidget(copyBtn);

    auto* copyAllBtn = new QPushButton(tr("复制全部信息"), this);
    connect(copyAllBtn, &QPushButton::clicked, this, [this]() {
        QString info;
        info += "文件名: " + m_nameLabel->text() + "\n";
        info += "存储位置: " + m_storageLabel->text() + "\n";
        info += "标签: " + m_tagsLabel->text() + "\n";
        info += "大小: " + m_sizeLabel->text() + "\n";
        info += "上传日期: " + m_uploadDateLabel->text() + "\n";
        info += "归属地: " + m_locationLabel->text() + "\n";
        info += "链接: " + m_urlEdit->toPlainText() + "\n";
        info += "描述: " + m_descEdit->toPlainText() + "\n";
        QApplication::clipboard()->setText(info);
    });
    btnLayout->addWidget(copyAllBtn);

    mainLayout->addLayout(btnLayout);
}

void ImageInfoDialog::queryImage(const QString& imageUrl) {
    m_loadingLabel->setVisible(true);
    m_loadingLabel->setText(tr("正在查询图片信息..."));
    m_loadingLabel->style()->unpolish(m_loadingLabel);
    m_loadingLabel->style()->polish(m_loadingLabel);

    // Extract filename from URL for querying img.scdn.io API
    QString filename = imageUrl.section('/', -1);

    // Query the scnd.io API
    QUrl url("https://img.scdn.io/api/v1.php");
    QUrlQuery query;
    query.addQueryItem("q", filename);
    url.setQuery(query);

    QNetworkRequest req(url);
    auto* reply = m_manager->get(req);

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        m_loadingLabel->setVisible(false);

        if (reply->error() != QNetworkReply::NoError) {
            int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            if (statusCode == 404) {
                m_loadingLabel->setText(tr("未在元数据服务中找到该图片。"));
            } else if (statusCode == 429) {
                m_loadingLabel->setText(tr("请求过于频繁，请稍后再试。"));
            } else {
                m_loadingLabel->setText(tr("查询失败: ") + reply->errorString());
            }
            return;
        }

        auto doc = QJsonDocument::fromJson(reply->readAll());
        auto obj = doc.object();

        if (obj["success"].toBool()) {
            auto data = obj["data"].toObject();
            m_currentInfo = ImageInfo::fromScndioJson(data);
            displayInfo(m_currentInfo);
        } else {
            m_loadingLabel->setText(obj["error"].toString(tr("未知错误")));
        }
    });
}

void ImageInfoDialog::displayInfo(const ImageInfo& info) {
    m_nameLabel->setText(info.originalFilename.isEmpty()
        ? info.filename : info.originalFilename + " → " + info.filename);
    m_storageLabel->setText(info.storageLocation + " (" + info.storageBackend + ")");
    m_tagsLabel->setText(info.tags.join(", "));

    if (info.originalSize > 0) {
        m_sizeLabel->setText(QString("%1 → %2（节省 %3%）")
            .arg(formatFileSize(info.originalSize))
            .arg(formatFileSize(info.compressedSize))
            .arg((info.originalSize - info.compressedSize) * 100 / info.originalSize));
    } else {
        m_sizeLabel->setText(info.sizeDisplay);
    }

    m_uploadDateLabel->setText(info.uploadDate);
    m_locationLabel->setText(info.location.isEmpty() ? tr("未知") : info.location);
    m_urlEdit->setPlainText(info.imageUrl.isEmpty() ? info.url : info.imageUrl);
    m_descEdit->setPlainText(info.contentDescription.isEmpty()
        ? tr("暂无描述信息。") : info.contentDescription);
}
