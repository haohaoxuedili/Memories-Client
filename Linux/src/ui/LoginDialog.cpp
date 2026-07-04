#include "ui/LoginDialog.h"
#include "app/Application.h"
#include "app/Settings.h"
#include "network/ApiClient.h"
#include "utils/Logger.h"
#include "ui/AppleTitleBar.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDesktopServices>
#include <QUrlQuery>
#include <QRandomGenerator>
#include <QMessageBox>
#include <QTcpSocket>
#include <QCryptographicHash>
#include <QTimer>

LoginDialog::LoginDialog(QWidget* parent)
    : QDialog(parent)
    , m_userLabel(new QLabel(this))
    , m_qqLabel(new QLabel(this))
    , m_tenantLabel(new QLabel(this))
    , m_loginBtn(new QPushButton(this))
    , m_logoutBtn(new QPushButton(tr("登出"), this))
    , m_statusLabel(new QLabel(this))
    , m_manager(new QNetworkAccessManager(this))
{
    setupUi();
    updateUi();
}

LoginDialog::~LoginDialog() {
    if (m_callbackServer) {
        m_callbackServer->close();
        delete m_callbackServer;
    }
}

void LoginDialog::setupUi() {
    setWindowTitle(tr("校园墙 OAuth 登录"));
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    resize(440, 460);
    setMinimumSize(400, 380);

    auto* outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(0,0,0,0);
    outerLayout->setSpacing(0);

    // Title bar: traffic lights only, title centered
    auto* titleBar = new AppleTitleBar(this, tr("账号管理"), false);
    outerLayout->addWidget(titleBar);

    auto* mainLayout = new QVBoxLayout();
    mainLayout->setSpacing(12);
    mainLayout->setContentsMargins(28, 20, 28, 24);
    outerLayout->addLayout(mainLayout);

    // User info section
    auto* infoGroup = new QGroupBox(tr("当前用户"));
    auto* infoLayout = new QVBoxLayout(infoGroup);
    infoLayout->setSpacing(12);

    // Avatar + name + QQ in one row
    auto* avatarRow = new QHBoxLayout();
    avatarRow->setSpacing(16);

    // QQ avatar
    auto* avatarCircle = new QLabel();
    avatarCircle->setFixedSize(56, 56);
    avatarCircle->setScaledContents(true);
    avatarCircle->setObjectName("userAvatar");
    avatarCircle->setStyleSheet(
        "QLabel#userAvatar { background: #E2E8F0; border-radius: 28px; "
        "border: 2px solid rgba(29,110,90,0.15); }");
    avatarRow->addWidget(avatarCircle);

    // Name + QQ column
    auto* nameCol = new QVBoxLayout();
    nameCol->setSpacing(2);
    m_userLabel->setStyleSheet("font-size: 17px; font-weight: 700; color: #0f172a;");
    nameCol->addWidget(m_userLabel);
    m_qqLabel->setStyleSheet("font-size: 13px; color: #64748b;");
    nameCol->addWidget(m_qqLabel);
    avatarRow->addLayout(nameCol, 1);
    infoLayout->addLayout(avatarRow);

    // Tenant
    auto* tenantRow = new QHBoxLayout();
    auto* tenantIcon = new QLabel("🏫");
    tenantIcon->setStyleSheet("font-size: 14px; background: transparent;");
    tenantRow->addWidget(tenantIcon);
    m_tenantLabel->setStyleSheet("font-size: 13px; color: #475569;");
    tenantRow->addWidget(m_tenantLabel, 1);
    infoLayout->addLayout(tenantRow);

    mainLayout->addWidget(infoGroup);

    // OAuth section
    auto* oauthGroup = new QGroupBox(tr("校园墙 OAuth"));
    auto* oauthLayout = new QVBoxLayout(oauthGroup);
    oauthLayout->setSpacing(12);

    auto* oauthDesc = new QLabel(
        tr("使用校园墙账号授权登录。\n点击按钮后将打开浏览器进行授权。"));
    oauthDesc->setWordWrap(true);
    oauthDesc->setStyleSheet("color: #64748b; font-size: 13px;");
    oauthLayout->addWidget(oauthDesc);

    m_loginBtn->setMinimumHeight(48);
    m_loginBtn->setCursor(Qt::PointingHandCursor);
    m_loginBtn->setStyleSheet(
        "QPushButton { font-size: 15px; font-weight: 700; "
        "background: #1D6E5A; color: white; border-radius: 12px; "
        "border: none; padding: 0px 24px; }"
        "QPushButton:hover { background: #175A48; }"
        "QPushButton:pressed { background: #124A3A; }");
    oauthLayout->addWidget(m_loginBtn);

    m_logoutBtn->setMinimumHeight(40);
    m_logoutBtn->setCursor(Qt::PointingHandCursor);
    m_logoutBtn->setStyleSheet(
        "QPushButton { font-size: 13px; font-weight: 600; color: #EF4444; "
        "background: rgba(239,68,68,0.06); border: 1px solid rgba(239,68,68,0.12); "
        "border-radius: 10px; padding: 0px 24px; }"
        "QPushButton:hover { background: rgba(239,68,68,0.12); }");
    oauthLayout->addWidget(m_logoutBtn);

    mainLayout->addWidget(oauthGroup);

    // Status
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->setWordWrap(true);
    mainLayout->addWidget(m_statusLabel);

    // Connections
    connect(m_loginBtn, &QPushButton::clicked, this, &LoginDialog::onStartLogin);
    connect(m_logoutBtn, &QPushButton::clicked, this, &LoginDialog::onLogout);
}

void LoginDialog::updateUi() {
    auto* s = Application::instance()->settings();
    bool loggedIn = s->isLoggedIn();

    // Update avatar - load QQ avatar
    auto* avatarLabel = findChild<QLabel*>("userAvatar");
    if (avatarLabel) {
        QString qq = s->userQq();
        if (!qq.isEmpty()) {
            QString avatarUrl = QString("https://q1.qlogo.cn/g?b=qq&nk=%1&s=100").arg(qq);
            auto* netMgr = new QNetworkAccessManager(this);
            QNetworkRequest req{QUrl(avatarUrl)};
            auto* reply = netMgr->get(req);
            connect(reply, &QNetworkReply::finished, avatarLabel, [avatarLabel, reply]() {
                reply->deleteLater();
                if (reply->error() == QNetworkReply::NoError) {
                    QPixmap pix;
                    pix.loadFromData(reply->readAll());
                    avatarLabel->setPixmap(pix.scaled(56, 56, Qt::KeepAspectRatio, Qt::SmoothTransformation));
                }
            });
        } else {
            avatarLabel->setStyleSheet(
                "QLabel#userAvatar { background: #E2E8F0; border-radius: 28px; "
                "border: 2px solid rgba(29,110,90,0.15); }");
        }
    }

    if (loggedIn) {
        m_userLabel->setText(s->userName());
        m_qqLabel->setText("QQ: " + s->userQq());
        m_tenantLabel->setText(s->userTenantName());
        m_loginBtn->setText(s->userName().isEmpty() ? tr("我的") : s->userName());
        m_logoutBtn->setVisible(true);
        m_statusLabel->setText(tr("✓ 已登录 — ") + s->userTenantName());
        m_statusLabel->setStyleSheet("color: #1D6E5A; font-weight: 600;");
    } else {
        m_userLabel->setText(tr("未登录"));
        m_qqLabel->setText("");
        m_tenantLabel->setText("-");
        m_loginBtn->setText(tr("校园墙 OAuth 登录"));
        m_logoutBtn->setVisible(false);
        m_statusLabel->setText(tr("点击按钮进行授权登录"));
        m_statusLabel->setStyleSheet("color: #94a3b8;");
    }
}

void LoginDialog::onStartLogin() {
    startOAuthFlow();
}

void LoginDialog::onLogout() {
    Application::instance()->settings()->clearSession();
    Application::instance()->settings()->save();
    Application::instance()->apiClient()->setAccessToken("");
    updateUi();
    LOG_INFO("User logged out");
}

// ---- OAuth Flow ----

void LoginDialog::startOAuthFlow() {
    m_statusLabel->setText(tr("正在准备授权..."));
    m_statusLabel->setStyleSheet("color: blue;");
    m_loginBtn->setEnabled(false);

    // Generate PKCE
    m_codeVerifier = createCodeVerifier();
    m_oauthState = QUuid::createUuid().toString(QUuid::WithoutBraces);

    // Build authorization URL
    QUrl url(OAuthConfig::AUTHORIZE_URL);
    QUrlQuery query;
    query.addQueryItem("response_type", "code");
    query.addQueryItem("client_id", OAuthConfig::CLIENT_ID);
    query.addQueryItem("redirect_uri", OAuthConfig::REDIRECT_URI);
    query.addQueryItem("scope", OAuthConfig::SCOPE);
    query.addQueryItem("state", m_oauthState);
    query.addQueryItem("code_challenge", createCodeChallenge(m_codeVerifier));
    query.addQueryItem("code_challenge_method", "S256");
    url.setQuery(query);

    // Start local callback server
    startCallbackServer();

    // Open browser
    m_statusLabel->setText(tr("请在浏览器中完成授权..."));
    QDesktopServices::openUrl(url);
}

void LoginDialog::startCallbackServer() {
    if (m_callbackServer) {
        m_callbackServer->close();
        delete m_callbackServer;
    }

    m_callbackServer = new QTcpServer(this);
    connect(m_callbackServer, &QTcpServer::newConnection, this, &LoginDialog::handleCallback);

    if (!m_callbackServer->listen(QHostAddress::LocalHost, OAuthConfig::CALLBACK_PORT)) {
        LOG_ERROR("Failed to start callback server on port " +
                  QString::number(OAuthConfig::CALLBACK_PORT));
        m_statusLabel->setText(tr("无法启动回调服务器，端口被占用"));
        m_statusLabel->setStyleSheet("color: red;");
        m_loginBtn->setEnabled(true);
        return;
    }

    LOG_INFO("OAuth callback server listening on port " +
             QString::number(OAuthConfig::CALLBACK_PORT));
}

void LoginDialog::handleCallback() {
    auto* socket = m_callbackServer->nextPendingConnection();
    if (!socket) return;

    connect(socket, &QTcpSocket::readyRead, this, [this, socket]() {
        QString request = socket->readAll();
        QString firstLine = request.section('\n', 0, 0);
        QString path = firstLine.section(' ', 1, 1);

        // Extract query params
        QString queryStr = path.section('?', 1);
        QUrlQuery query(queryStr);
        QString code = query.queryItemValue("code");
        QString state = query.queryItemValue("state");

        // Send response
        QString html = "<!DOCTYPE html><html><head><meta charset='utf-8'><meta name='viewport' content='width=device-width,initial-scale=1'>"
                       "<title>授权完成</title><style>body{font-family:sans-serif;display:flex;align-items:center;justify-content:center;"
                       "height:100vh;margin:0;background:#f5f5f5;color:#333;text-align:center}"
                       "h2{color:#1D6E5A}</style></head><body><div><h2>✓ 授权完成</h2><p>请返回回忆映像客户端。</p>"
                       "<p style='color:#999;font-size:13px'>此页面可安全关闭</p></div>"
                       "<script>setTimeout(function(){window.close();},600);</script></body></html>";
        QByteArray body = html.toUtf8();
        QString header = "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=utf-8\r\n"
                         "Content-Length: " + QString::number(body.size()) + "\r\nConnection: close\r\n\r\n";
        socket->write(header.toUtf8());
        socket->write(body);
        socket->flush();
        socket->waitForBytesWritten(2000);
        socket->close();

        // Validate state
        if (state != m_oauthState || code.isEmpty()) {
            m_statusLabel->setText(tr("授权回调无效"));
            m_statusLabel->setStyleSheet("color: red;");
            m_loginBtn->setEnabled(true);
            return;
        }

        LOG_INFO("OAuth callback received, exchanging code...");
        m_statusLabel->setText(tr("正在交换令牌..."));
        exchangeCodeForToken(code);
    });
}

// ---- PKCE ----

QString LoginDialog::createCodeVerifier() {
    QByteArray bytes(32, 0);
    for (int i = 0; i < 32; ++i) {
        bytes[i] = static_cast<char>(QRandomGenerator::global()->bounded(256));
    }
    return bytes.toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);
}

QString LoginDialog::createCodeChallenge(const QString& verifier) {
    QByteArray hash = QCryptographicHash::hash(
        verifier.toUtf8(), QCryptographicHash::Sha256);
    return hash.toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);
}

// ---- Token Exchange ----

void LoginDialog::exchangeCodeForToken(const QString& code) {
    QNetworkRequest req{QUrl(OAuthConfig::TOKEN_URL)};
    req.setHeader(QNetworkRequest::ContentTypeHeader,
                  "application/x-www-form-urlencoded");

    QUrlQuery body;
    body.addQueryItem("grant_type", "authorization_code");
    body.addQueryItem("code", code);
    body.addQueryItem("redirect_uri", OAuthConfig::REDIRECT_URI);
    body.addQueryItem("client_id", OAuthConfig::CLIENT_ID);
    body.addQueryItem("code_verifier", m_codeVerifier);
    body.addQueryItem("client_secret", OAuthConfig::CLIENT_SECRET);

    auto* reply = m_manager->post(req, body.toString(QUrl::FullyEncoded).toUtf8());
    connect(reply, &QNetworkReply::finished, this, [this]() {
        auto* finishedReply = qobject_cast<QNetworkReply*>(sender());
        finishedReply->deleteLater();

        if (finishedReply->error() != QNetworkReply::NoError) {
            m_statusLabel->setText(tr("令牌交换失败: ") + finishedReply->errorString());
            m_statusLabel->setStyleSheet("color: red;");
            m_loginBtn->setEnabled(true);
            return;
        }

        auto doc = QJsonDocument::fromJson(finishedReply->readAll());
        auto obj = doc.object();
        QString accessToken = obj["access_token"].toString();

        if (accessToken.isEmpty()) {
            m_statusLabel->setText(tr("未获取到访问令牌: ") +
                obj["error"].toString(finishedReply->errorString()));
            m_statusLabel->setStyleSheet("color: red;");
            m_loginBtn->setEnabled(true);
            return;
        }

        LOG_INFO("Token obtained, fetching user info...");
        m_statusLabel->setText(tr("正在获取用户信息..."));
        fetchUserInfo(accessToken);
    });
}

void LoginDialog::fetchUserInfo(const QString& accessToken) {
    QNetworkRequest req{QUrl(OAuthConfig::USERINFO_URL)};
    req.setRawHeader("Authorization", ("Bearer " + accessToken).toUtf8());

    auto* reply = m_manager->get(req);
    connect(reply, &QNetworkReply::finished, this, [this, accessToken]() {
        auto* finishedReply = qobject_cast<QNetworkReply*>(sender());
        finishedReply->deleteLater();

        if (finishedReply->error() != QNetworkReply::NoError) {
            m_statusLabel->setText(tr("获取用户信息失败: ") + finishedReply->errorString());
            m_statusLabel->setStyleSheet("color: red;");
            m_loginBtn->setEnabled(true);
            return;
        }

        auto doc = QJsonDocument::fromJson(finishedReply->readAll());
        auto obj = doc.object();

        if (obj.contains("error")) {
            m_statusLabel->setText(tr("获取用户信息失败: ") + obj["error"].toString());
            m_statusLabel->setStyleSheet("color: red;");
            m_loginBtn->setEnabled(true);
            return;
        }

        saveSession(obj, accessToken);
    });
}

void LoginDialog::saveSession(const QJsonObject& userInfo, const QString& accessToken) {
    auto* s = Application::instance()->settings();
    s->setAccessToken(accessToken);
    s->setUserSub(userInfo["sub"].toString());
    s->setUserQq(userInfo["name"].toString());
    s->setUserName(userInfo["username"].toString(
        userInfo["name"].toString())); // fallback to QQ number
    s->setUserTenantName(userInfo["tenant_name"].toString());
    s->setUserTenantSlug(userInfo["tenant_slug"].toString());
    s->save();

    Application::instance()->apiClient()->setAccessToken(accessToken);

    m_statusLabel->setText(tr("✓ 登录成功！"));
    m_statusLabel->setStyleSheet("color: green;");
    m_loginBtn->setEnabled(true);
    updateUi();

    LOG_INFO("Login successful: " + s->userName() + " (QQ: " + s->userQq() + ")");

    emit loginSuccess();
}
