#include "ui/MainWindow.h"
#include "ui/GalleryWidget.h"
#include "ui/ImageViewer.h"
#include "ui/UploadDialog.h"
#include "ui/SettingsDialog.h"
#include "ui/LoginDialog.h"
#include "ui/MessageBox.h"
#include "app/Application.h"
#include "app/Settings.h"
#include "network/HealthChecker.h"
#include "network/ApiClient.h"
#include "utils/Logger.h"
#include "utils/ThemeManager.h"

#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QMenu>
#include <QAction>
#include <QCloseEvent>
#include <QMouseEvent>
#include <QMessageBox>
#include <QApplication>
#include <QtPrintSupport/QPrintDialog>
#include <QtPrintSupport/QPrinter>
#include <QFile>
#include <QTimer>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QToolButton>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_stack(new QStackedWidget(this))
    , m_gallery(nullptr)
    , m_viewer(nullptr)
    , m_uploadDlg(nullptr)
    , m_settingsDlg(nullptr)
    , m_loginDlg(nullptr)
    , m_titleBar(nullptr)
    , m_statusLabel(new QLabel(this))
    , m_healthLabel(new QLabel(this))
    , m_avatarLabel(new QLabel(this))
    , m_trayIcon(nullptr)
{
    setupUi();
    applyTheme();
    checkServiceHealth();
    updateUserDisplay();
}

MainWindow::~MainWindow() = default;

void MainWindow::setupUi() {
    setWindowTitle("Memories");
    setWindowIcon(QIcon(":/icons/app.svg"));
    resize(1280, 800);
    setMinimumSize(800, 600);

    // Main container: title bar + content
    auto* mainContainer = new QWidget(this);
    auto* mainVLayout = new QVBoxLayout(mainContainer);
    mainVLayout->setContentsMargins(0, 0, 0, 0);
    mainVLayout->setSpacing(0);
    setCentralWidget(mainContainer);

    // Suppress QMainWindow's built-in menu bar
    QMainWindow::setMenuBar(new QMenuBar(this));
    QMainWindow::menuBar()->setVisible(false);

    setupTitleBar();
    mainVLayout->addWidget(m_titleBar);

    auto* shell = new QWidget(this);
    shell->setObjectName("androidShell");
    auto* shellLayout = new QHBoxLayout(shell);
    shellLayout->setContentsMargins(18, 16, 18, 16);
    shellLayout->setSpacing(16);
    shellLayout->addWidget(setupSideNavigation());

    m_stack->addWidget(m_gallery = new GalleryWidget(this));
    m_stack->addWidget(m_viewer = new ImageViewer(this));
    m_stack->addWidget(m_uploadDlg = new UploadDialog(this, true));
    m_stack->addWidget(m_loginDlg = new LoginDialog(this, true));
    m_stack->setCurrentWidget(m_gallery);
    m_stack->setObjectName("contentStack");
    shellLayout->addWidget(m_stack, 1);
    mainVLayout->addWidget(shell, 1);

    // Navigation
    connect(m_gallery, &GalleryWidget::imageSelected, this, &MainWindow::showImageViewer);
    connect(m_viewer, &ImageViewer::backToGallery, this, &MainWindow::showGallery);
    connect(m_loginDlg, &LoginDialog::loginSuccess, this, [this]() {
        updateUserDisplay();
    });
    setActiveNavigation(m_galleryNavButton);

    setupStatusBar();
    setupSystemTray();

    // Require login on startup
    if (!Application::instance()->settings()->isLoggedIn()) {
        QTimer::singleShot(200, this, [this]() {
            auto* dlg = new LoginDialog(this);
            dlg->setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
            dlg->setModal(true);
            connect(dlg, &LoginDialog::loginSuccess, this, [this, dlg]() {
                updateUserDisplay();
                dlg->accept();
            });
            if (dlg->exec() != QDialog::Accepted) {
                QTimer::singleShot(0, qApp, &QApplication::quit);
            }
            dlg->deleteLater();
        });
    }
}

void MainWindow::setupTitleBar() {
    m_titleBar = new AppleTitleBar(this, "Memories", true);
    m_titleBar->setIcon(QIcon(":/icons/app.svg"));
}

// ---- Window dragging ----

void MainWindow::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton && m_titleBar->geometry().contains(event->pos())) {
        m_draggingWindow = true;
        m_dragPos = event->globalPosition().toPoint() - frameGeometry().topLeft();
    }
}

void MainWindow::mouseMoveEvent(QMouseEvent* event) {
    if (m_draggingWindow && (event->buttons() & Qt::LeftButton)) {
        if (isMaximized()) showNormal();
        move(event->globalPosition().toPoint() - m_dragPos);
    }
}

void MainWindow::mouseReleaseEvent(QMouseEvent* event) {
    Q_UNUSED(event);
    m_draggingWindow = false;
}

void MainWindow::mouseDoubleClickEvent(QMouseEvent* event) {
    if (m_titleBar && m_titleBar->geometry().contains(event->pos())) {
        if (isMaximized()) showNormal();
        else showMaximized();
    }
}

void MainWindow::setupMenuBar(QMenuBar* menuBar) {
    auto* fileMenu = menuBar->addMenu(tr("文件(&F)"));

    auto* uploadAction = fileMenu->addAction(tr("上传图片(&U)..."));
    uploadAction->setShortcut(QKeySequence("Ctrl+U"));
    connect(uploadAction, &QAction::triggered, this, &MainWindow::showUploadDialog);

    fileMenu->addSeparator();

    auto* settingsAction = fileMenu->addAction(tr("设置(&S)..."));
    settingsAction->setShortcut(QKeySequence("Ctrl+,"));
    connect(settingsAction, &QAction::triggered, this, &MainWindow::showSettingsDialog);

    fileMenu->addSeparator();

    auto* exitAction = fileMenu->addAction(tr("退出(&X)"));
    exitAction->setShortcut(QKeySequence("Ctrl+Q"));
    connect(exitAction, &QAction::triggered, this, &QWidget::close);

    auto* viewMenu = menuBar->addMenu(tr("视图(&V)"));

    auto* galleryAction = viewMenu->addAction(tr("图片广场(&G)"));
    galleryAction->setShortcut(QKeySequence("Ctrl+G"));
    connect(galleryAction, &QAction::triggered, this, &MainWindow::showGallery);

    auto* refreshAction = viewMenu->addAction(tr("刷新(&R)"));
    refreshAction->setShortcut(QKeySequence("F5"));
    connect(refreshAction, &QAction::triggered, m_gallery, &GalleryWidget::loadImages);

    auto* accountMenu = menuBar->addMenu(tr("账号(&A)"));
    m_accountMenu = accountMenu;

    m_accountLoginAction = accountMenu->addAction(tr("登录(&L)..."));
    connect(m_accountLoginAction, &QAction::triggered, this, &MainWindow::showLoginDialog);

    m_accountLogoutAction = accountMenu->addAction(tr("登出(&O)"));
    m_accountLogoutAction->setVisible(false);
    connect(m_accountLogoutAction, &QAction::triggered, this, [this]() {
        Application::instance()->settings()->clearSession();
        Application::instance()->settings()->save();
        Application::instance()->apiClient()->setAccessToken("");
        updateUserDisplay();
    });

    auto* helpMenu = menuBar->addMenu(tr("帮助(&H)"));
    auto* aboutAction = helpMenu->addAction(tr("关于(&A)"));
    connect(aboutAction, &QAction::triggered, this, [this]() {
        MessageBox::about(this, tr("关于 Memories"),
            tr("Memories v1.1.0\n跨平台图片管理与分享客户端。"));
    });
}

void MainWindow::setupToolBar(QToolBar* toolbar) {
    toolbar->setMovable(false);
    toolbar->setIconSize(QSize(24, 24));

    auto* galleryAction = toolbar->addAction(QIcon(":/icons/ic_gallery.svg"), tr("广场"));
    connect(galleryAction, &QAction::triggered, this, &MainWindow::showGallery);

    auto* uploadAction = toolbar->addAction(QIcon(":/icons/ic_upload.svg"), tr("上传"));
    connect(uploadAction, &QAction::triggered, this, &MainWindow::showUploadDialog);

    toolbar->addSeparator();

    // Avatar label
    m_avatarLabel->setFixedSize(28, 28);
    m_avatarLabel->setAlignment(Qt::AlignCenter);
    m_avatarLabel->setStyleSheet(
        "QLabel { background: qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 #F8FAFC,stop:0.48 #BFE7E2,stop:1 #BFD9FF); color: #123235; border-radius: 14px; "
        "border: 1px solid rgba(255,255,255,0.82); font-weight: 800; font-size: 12px; min-width: 28px; min-height: 28px; }");
    m_avatarLabel->setVisible(false);
    toolbar->addWidget(m_avatarLabel);

    auto* loginAction = toolbar->addAction(QIcon(":/icons/ic_profile.svg"), tr("登录"));
    loginAction->setObjectName("toolbarLoginBtn");
    connect(loginAction, &QAction::triggered, this, [this]() {
        if (Application::instance()->settings()->isLoggedIn()) {
            showLoginDialog(); // shows account info when logged in
        } else {
            showLoginDialog();
        }
    });
}

QFrame* MainWindow::setupSideNavigation() {
    auto* sideNav = new QFrame(this);
    sideNav->setObjectName("sideNavigation");
    sideNav->setFixedWidth(112);

    auto* navLayout = new QVBoxLayout(sideNav);
    navLayout->setContentsMargins(12, 14, 12, 14);
    navLayout->setSpacing(10);

    m_galleryNavButton = createNavButton(tr("广场"), QIcon(":/icons/ic_gallery.svg"));
    connect(m_galleryNavButton, &QToolButton::clicked, this, &MainWindow::showGallery);
    navLayout->addWidget(m_galleryNavButton);

    m_uploadNavButton = createNavButton(tr("上传"), QIcon(":/icons/ic_upload.svg"));
    connect(m_uploadNavButton, &QToolButton::clicked, this, [this]() {
        setActiveNavigation(m_uploadNavButton);
        showUploadDialog();
    });
    navLayout->addWidget(m_uploadNavButton);

    navLayout->addStretch();

    m_profileNavButton = createNavButton(tr("个人"), QIcon(":/icons/ic_profile.svg"));
    connect(m_profileNavButton, &QToolButton::clicked, this, [this]() {
        setActiveNavigation(m_profileNavButton);
        showLoginDialog();
    });
    navLayout->addWidget(m_profileNavButton);

    return sideNav;
}

QToolButton* MainWindow::createNavButton(const QString& text, const QIcon& icon) {
    auto* button = new QToolButton(this);
    button->setProperty("sideNavButton", true);
    button->setText(text);
    button->setIcon(icon);
    button->setIconSize(QSize(28, 28));
    button->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    button->setCheckable(true);
    button->setAutoRaise(false);
    button->setCursor(Qt::PointingHandCursor);
    button->setFixedSize(88, 72);
    return button;
}

void MainWindow::setActiveNavigation(QToolButton* activeButton) {
    const QList<QToolButton*> buttons = {m_galleryNavButton, m_uploadNavButton, m_profileNavButton};
    for (auto* button : buttons) {
        if (!button) continue;
        button->setChecked(button == activeButton);
        button->style()->unpolish(button);
        button->style()->polish(button);
    }
}

void MainWindow::setupStatusBar() {
    m_healthLabel->setProperty("statusPill", "checking");
    m_statusLabel->setProperty("statusText", true);
    statusBar()->addPermanentWidget(m_healthLabel);
    statusBar()->addPermanentWidget(m_statusLabel, 1);
    m_statusLabel->setText(tr("就绪"));
    m_healthLabel->setText(tr("● 检测中..."));
}

void MainWindow::setupSystemTray() {
    m_trayIcon = new QSystemTrayIcon(
        QIcon(":/icons/app.svg"), this);
    m_trayIcon->setToolTip("Memories");

    auto* trayMenu = new QMenu(this);
    trayMenu->addAction(tr("显示主窗口"), this, &QWidget::show);
    trayMenu->addAction(tr("上传图片..."), this, &MainWindow::showUploadDialog);
    trayMenu->addSeparator();
    trayMenu->addAction(tr("退出"), qApp, &QApplication::quit);

    m_trayIcon->setContextMenu(trayMenu);
    connect(m_trayIcon, &QSystemTrayIcon::activated, this, [this](QSystemTrayIcon::ActivationReason reason) {
        if (reason == QSystemTrayIcon::Trigger || reason == QSystemTrayIcon::DoubleClick) {
            if (isVisible()) {
                hide();
            } else {
                show();
                raise();
                activateWindow();
            }
        }
    });
    m_trayIcon->show();
}

void MainWindow::applyTheme() {
    QString theme = Application::instance()->settings()->theme();
    ThemeManager::instance().setTheme(theme);

    bool isDark = (theme == "dark");
    QString qss = ThemeManager::instance().buildStylesheet(isDark);
    qApp->setStyleSheet(qss);
}

void MainWindow::checkServiceHealth() {
    m_healthLabel->setText(tr("● Checking..."));
    m_healthLabel->setProperty("statusPill", "checking");
    m_healthLabel->style()->unpolish(m_healthLabel);
    m_healthLabel->style()->polish(m_healthLabel);
    auto* checker = Application::instance()->healthChecker();
    connect(checker, &HealthChecker::healthOk, this, [this]() {
        m_healthLabel->setText(tr("● 已连接"));
        m_healthLabel->setProperty("statusPill", "ok");
        m_healthLabel->style()->unpolish(m_healthLabel);
        m_healthLabel->style()->polish(m_healthLabel);
        m_statusLabel->setText(tr("服务运行正常"));
    });
    connect(checker, &HealthChecker::healthFail, this, [this](const QString& err) {
        m_healthLabel->setText(tr("● 未连接"));
        m_healthLabel->setProperty("statusPill", "error");
        m_healthLabel->style()->unpolish(m_healthLabel);
        m_healthLabel->style()->polish(m_healthLabel);
        m_statusLabel->setText(tr("服务不可达: ") + err);
    });
    checker->ping();
}

void MainWindow::onHealthCheckResult(bool ok) {
    if (ok) {
        m_gallery->loadImages();
    }
}

void MainWindow::showGallery() {
    m_stack->setCurrentWidget(m_gallery);
    setActiveNavigation(m_galleryNavButton);
}

void MainWindow::showImageViewer(const QString& imageUrl) {
    // Build image list from gallery
    QStringList urls;
    int currentIdx = 0;
    for (const auto& info : m_gallery->images()) {
        urls.append(info.url);
    }
    currentIdx = urls.indexOf(imageUrl);
    if (currentIdx < 0) currentIdx = 0;

    m_viewer->setImageList(urls, currentIdx);
    m_viewer->loadImage(imageUrl);
    m_stack->setCurrentWidget(m_viewer);
    setActiveNavigation(m_galleryNavButton);
}

void MainWindow::showUploadDialog() {
    m_stack->setCurrentWidget(m_uploadDlg);
    setActiveNavigation(m_uploadNavButton);
}

void MainWindow::showSettingsDialog() {
    if (!m_settingsDlg) {
        m_settingsDlg = new SettingsDialog(this);
    }
    m_settingsDlg->show();
    m_settingsDlg->raise();
}

void MainWindow::showLoginDialog() {
    m_stack->setCurrentWidget(m_loginDlg);
    setActiveNavigation(m_profileNavButton);
}

void MainWindow::onLoginSuccess() {
    updateUserDisplay();
}

void MainWindow::updateUserDisplay() {
    auto* s = Application::instance()->settings();
    if (s->isLoggedIn()) {
        QString username = s->userName();
        m_statusLabel->setText(username + " (QQ: " + s->userQq() + ")");
        setWindowTitle("Memories - " + username + " @" + s->userTenantName());

        m_avatarLabel->setText(username.isEmpty() ? "?" : username.left(1));
        m_avatarLabel->setVisible(true);

        if (m_accountLoginAction) m_accountLoginAction->setText(username.isEmpty() ? tr("我的") : username);
        if (m_accountLogoutAction) m_accountLogoutAction->setVisible(true);
    } else {
        m_statusLabel->setText(tr("就绪 - 未登录"));
        setWindowTitle("Memories");
        m_avatarLabel->setVisible(false);

        if (m_accountLoginAction) m_accountLoginAction->setText(tr("登录(&L)..."));
        if (m_accountLogoutAction) m_accountLogoutAction->setVisible(false);
    }
}

void MainWindow::closeEvent(QCloseEvent* event) {
    if (m_trayIcon && m_trayIcon->isVisible()) {
        hide();
        m_trayIcon->showMessage("Memories",
            tr("应用已最小化到系统托盘"), QSystemTrayIcon::Information, 2000);
        event->ignore();
    } else {
        event->accept();
    }
}
