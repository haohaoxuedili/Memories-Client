#include "utils/ThemeManager.h"
#include <QFile>
#include <QTextStream>

ThemeManager& ThemeManager::instance() {
    static ThemeManager inst;
    return inst;
}

ThemeManager::ThemeManager() {
    initThemes();
    m_current = "mint";
}

void ThemeManager::initThemes() {
    // ---- XMail default mist teal ----
    m_themes["mint"] = {
        "晨雾青", "rgba(54,179,168,0.072)", "rgba(119,172,213,0.058)",
        "rgba(255,255,255,0.90)", "rgba(54,179,168,0.105)", "16px",
        "#102326", "#3f5961", "#7d9399",
        "#1f9f93", "#167fbc", "rgba(54,179,168,0.11)", "rgba(119,172,213,0.13)", "rgba(54,179,168,0.075)",
        "rgba(255,255,255,0.82)", "rgba(255,255,255,0.86)",
        "rgba(255,255,255,0.76)", "rgba(255,255,255,0.92)",
        "rgba(54,179,168,0.13)", "rgba(119,172,213,0.12)",
        "rgba(255,255,255,0.96)", "rgba(246,253,252,0.78)"
    };

    // ---- XMail rose gold ----
    m_themes["rose"] = {
        "玫瑰金", "rgba(244,114,182,0.070)", "rgba(249,168,212,0.060)",
        "rgba(255,255,255,0.90)", "rgba(244,114,182,0.10)", "16px",
        "#2f1724", "#6b4054", "#a98295",
        "#f472b6", "#ec4899", "rgba(244,114,182,0.10)", "rgba(249,168,212,0.14)", "rgba(244,114,182,0.08)",
        "rgba(255,255,255,0.82)", "rgba(255,255,255,0.86)",
        "rgba(255,255,255,0.76)", "rgba(255,255,255,0.92)",
        "rgba(244,114,182,0.13)", "rgba(249,168,212,0.12)",
        "rgba(255,255,255,0.96)", "rgba(255,248,252,0.78)"
    };

    // ---- XMail coral orange ----
    m_themes["sky"] = {
        "珊瑚橙", "rgba(251,113,133,0.070)", "rgba(253,164,175,0.060)",
        "rgba(255,255,255,0.90)", "rgba(251,113,133,0.10)", "16px",
        "#331820", "#704251", "#ad8290",
        "#fb7185", "#f43f5e", "rgba(251,113,133,0.10)", "rgba(253,164,175,0.14)", "rgba(251,113,133,0.08)",
        "rgba(255,255,255,0.82)", "rgba(255,255,255,0.86)",
        "rgba(255,255,255,0.76)", "rgba(255,255,255,0.92)",
        "rgba(251,113,133,0.13)", "rgba(253,164,175,0.12)",
        "rgba(255,255,255,0.96)", "rgba(255,248,249,0.78)"
    };

    // ---- XMail lavender ----
    m_themes["lavender"] = {
        "薰衣草", "rgba(139,92,246,0.070)", "rgba(167,139,250,0.060)",
        "rgba(255,255,255,0.90)", "rgba(139,92,246,0.10)", "16px",
        "#24163f", "#594878", "#998bb7",
        "#8b5cf6", "#7c3aed", "rgba(139,92,246,0.10)", "rgba(167,139,250,0.14)", "rgba(139,92,246,0.08)",
        "rgba(255,255,255,0.82)", "rgba(255,255,255,0.86)",
        "rgba(255,255,255,0.76)", "rgba(255,255,255,0.92)",
        "rgba(139,92,246,0.13)", "rgba(167,139,250,0.12)",
        "rgba(255,255,255,0.96)", "rgba(251,249,255,0.78)"
    };

    // ---- XMail milk tea ----
    m_themes["sunset"] = {
        "奶茶色", "rgba(212,165,116,0.080)", "rgba(230,194,166,0.070)",
        "rgba(255,255,255,0.90)", "rgba(212,165,116,0.11)", "16px",
        "#332519", "#6d5845", "#aa9681",
        "#d4a574", "#b8956a", "rgba(212,165,116,0.11)", "rgba(230,194,166,0.15)", "rgba(212,165,116,0.09)",
        "rgba(255,255,255,0.82)", "rgba(255,255,255,0.86)",
        "rgba(255,255,255,0.76)", "rgba(255,255,255,0.92)",
        "rgba(212,165,116,0.14)", "rgba(230,194,166,0.13)",
        "rgba(255,255,255,0.96)", "rgba(255,250,246,0.78)"
    };

    // ---- Soft blue companion ----
    m_themes["ocean"] = {
        "清透蓝", "rgba(14,165,233,0.070)", "rgba(99,102,241,0.055)",
        "rgba(255,255,255,0.90)", "rgba(14,165,233,0.10)", "16px",
        "#102538", "#36566c", "#7f99aa",
        "#0ea5e9", "#0284c7", "rgba(14,165,233,0.10)", "rgba(99,102,241,0.12)", "rgba(14,165,233,0.08)",
        "rgba(255,255,255,0.82)", "rgba(255,255,255,0.86)",
        "rgba(255,255,255,0.76)", "rgba(255,255,255,0.92)",
        "rgba(14,165,233,0.13)", "rgba(99,102,241,0.10)",
        "rgba(255,255,255,0.96)", "rgba(247,252,255,0.78)"
    };
}

void ThemeManager::setTheme(const QString& name) {
    if (m_themes.contains(name)) {
        m_current = name;
    }
}

QString ThemeManager::currentTheme() const { return m_current; }

QStringList ThemeManager::themeNames() const {
    QStringList names;
    for (auto it = m_themes.begin(); it != m_themes.end(); ++it)
        names.append(it.key());
    return names;
}

const ThemeColors& ThemeManager::current() const {
    static ThemeColors fallback;
    auto it = m_themes.constFind(m_current);
    return it != m_themes.constEnd() ? *it : fallback;
}

QStringList ThemeManager::fontPresets() {
    return {
        "system", "noto", "wenquanyi", "source-han", "pingfang", "inter", "jetbrains"
    };
}

QString ThemeManager::fontPresetDisplayName(const QString& key) {
    if (key == "system") return "系统默认";
    if (key == "noto") return "Noto Sans SC";
    if (key == "wenquanyi") return "WenQuanYi Micro Hei";
    if (key == "source-han") return "Source Han Sans SC";
    if (key == "pingfang") return "PingFang SC";
    if (key == "inter") return "Inter";
    if (key == "jetbrains") return "JetBrains Mono";
    return key;
}

QString ThemeManager::buildStylesheet(bool dark) const {
    const auto& c = current();
    auto color = [&c](const QString& key) -> QString {
        if (key == "bg") return c.bgGradientEnd;
        if (key == "card") return c.cardBg;
        if (key == "text") return c.textPrimary;
        if (key == "text2") return c.textSecondary;
        if (key == "muted") return c.textMuted;
        if (key == "acc") return c.accent;
        if (key == "acch") return c.accentHover;
        if (key == "accl") return c.accentLight;
        return "";
    };

    if (dark) {
        // Optimized dark theme
        return QString(R"(
* { font-family: "%1"; font-size: 13px; color: #e4edf6; selection-background-color: rgba(141,198,255,0.28); }
QMainWindow { background: qradialgradient(cx:0.20,cy:0.10,radius:1.10,fx:0.20,fy:0.10,stop:0 #263447,stop:0.34 #14252e,stop:0.68 #101820,stop:1 #0a0f14); }
QWidget#androidShell { background: transparent; }
QStackedWidget#contentStack { background: qradialgradient(cx:0.42,cy:0.18,radius:0.98,fx:0.42,fy:0.18,stop:0 rgba(36,48,64,0.88),stop:0.54 rgba(19,28,39,0.78),stop:1 rgba(10,15,20,0.68)); border: 1px solid rgba(189,213,240,0.11); border-radius: 22px; }
QFrame#sideNavigation { background: qradialgradient(cx:0.50,cy:0.08,radius:0.94,fx:0.50,fy:0.08,stop:0 rgba(45,59,77,0.86),stop:0.55 rgba(20,30,42,0.76),stop:1 rgba(13,18,25,0.70)); border: 1px solid rgba(189,213,240,0.12); border-radius: 22px; }
QToolButton[sideNavButton="true"] { background: transparent; border: 1px solid transparent; border-radius: 16px; padding: 8px 6px; color: #9fb1c6; font-weight: 800; }
QToolButton[sideNavButton="true"]:hover { background: rgba(255,255,255,0.07); border-color: rgba(189,213,240,0.10); color: #e8f2ff; }
QToolButton[sideNavButton="true"]:checked { background: qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 rgba(255,255,255,0.16),stop:0.50 rgba(141,198,255,0.17),stop:1 rgba(72,187,173,0.16)); border-color: rgba(210,230,255,0.26); color: #ffffff; }
QMenuBar { background: rgba(12,18,25,0.76); border-bottom: 1px solid rgba(255,255,255,0.06); padding: 5px 10px; }
QMenuBar::item { padding: 7px 15px; margin: 1px 2px; border-radius: 9px; color: #9fb1c6; font-weight: 600; }
QMenuBar::item:selected { background: rgba(141,198,255,0.11); color: #d9ecff; }
QMenu { background: rgba(16,24,34,0.97); border: 1px solid rgba(189,213,240,0.12); border-radius: 14px; padding: 8px; }
QMenu::item { padding: 9px 34px 9px 16px; margin: 2px; border-radius: 9px; color: #aebdd0; }
QMenu::item:selected { background: rgba(141,198,255,0.13); color: #eef7ff; }
QToolBar { background: rgba(12,18,25,0.58); border-bottom: 1px solid rgba(255,255,255,0.06); padding: 7px 10px; spacing: 8px; }
QToolBar QToolButton { padding: 7px 14px; border-radius: 10px; color: #cbd5e1; font-weight: 700; border: 1px solid rgba(189,213,240,0.10); background: rgba(255,255,255,0.045); }
QToolBar QToolButton:hover { background: rgba(141,198,255,0.11); color: #f8fbff; border-color: rgba(141,198,255,0.24); }
QPushButton { padding: 9px 20px; border-radius: 10px; font-weight: 700; border: 1px solid rgba(148,163,184,0.14); min-height: 22px; background: rgba(255,255,255,0.045); color: #cbd5e1; }
QPushButton:hover { background: rgba(255,255,255,0.075); border-color: rgba(156,199,255,0.24); color: #f8fafc; }
QPushButton:disabled { color: rgba(203,213,225,0.38); background: rgba(255,255,255,0.03); border-color: rgba(255,255,255,0.04); }
QPushButton#primaryBtn, QPushButton[primaryBtn="true"] { background: qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 #F8FAFC,stop:0.46 #B9D7FF,stop:1 #B8B7FF); color: #111827; font-weight: 800; border: 1px solid rgba(255,255,255,0.30); }
QPushButton#primaryBtn:hover, QPushButton[primaryBtn="true"]:hover { background: qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 #FFFFFF,stop:0.48 #CDE4FF,stop:1 #CBC7FF); border-color: rgba(255,255,255,0.46); }
QPushButton[flat="true"] { background: rgba(255,255,255,0.045); border: 1px solid rgba(148,163,184,0.12); color: #aebdd0; }
QPushButton[flat="true"]:hover { background: rgba(156,199,255,0.10); border-color: rgba(156,199,255,0.24); color: #e5efff; }
QLineEdit, QComboBox, QSpinBox { padding: 9px 14px; border-radius: 11px; border: 1px solid rgba(189,213,240,0.12); background: rgba(9,14,20,0.52); color: #e4edf6; }
QLineEdit:hover, QComboBox:hover, QSpinBox:hover { border-color: rgba(141,198,255,0.22); background: rgba(13,20,28,0.62); }
QLineEdit:focus, QComboBox:focus, QSpinBox:focus { border-color: rgba(141,198,255,0.46); background: rgba(13,20,28,0.72); }
QComboBox::drop-down { border: none; width: 24px; subcontrol-origin: padding; subcontrol-position: right center; }
QComboBox::down-arrow { width: 10px; height: 10px; }
QComboBox QAbstractItemView { background: rgba(16,24,34,0.97); border: 1px solid rgba(189,213,240,0.12); border-radius: 10px; color: #e4edf6; padding: 5px; }
QComboBox QAbstractItemView::item { padding: 8px 14px; border-radius: 6px; }
QComboBox QAbstractItemView::item:selected { background: rgba(141,198,255,0.14); color: #eef7ff; }
QComboBox[uploadOptionCombo="true"] { min-height: 24px; padding: 9px 38px 9px 14px; border-radius: 14px; border: 1px solid rgba(141,198,255,0.24); background: qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 rgba(141,198,255,0.16),stop:0.48 rgba(72,187,173,0.12),stop:1 rgba(216,199,255,0.12)); color: #eef7ff; font-weight: 800; }
QComboBox[uploadOptionCombo="true"]:hover { border-color: rgba(141,198,255,0.40); background: qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 rgba(141,198,255,0.24),stop:0.48 rgba(72,187,173,0.18),stop:1 rgba(216,199,255,0.18)); }
QComboBox[uploadOptionCombo="true"]:focus { border-color: rgba(170,215,255,0.62); background: qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 rgba(141,198,255,0.30),stop:0.48 rgba(72,187,173,0.22),stop:1 rgba(216,199,255,0.22)); }
QComboBox[uploadOptionCombo="true"]::drop-down { width: 34px; border: none; border-left: 1px solid rgba(189,235,255,0.18); border-top-right-radius: 14px; border-bottom-right-radius: 14px; background: rgba(141,198,255,0.09); }
QComboBox[uploadOptionCombo="true"]::down-arrow { image: none; width: 0; height: 0; border-left: 5px solid transparent; border-right: 5px solid transparent; border-top: 6px solid #bfe3ff; margin-right: 12px; }
QComboBox[uploadOptionCombo="true"] QAbstractItemView, QAbstractItemView[uploadOptionPopup="true"] { background: qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 rgba(88,142,184,0.94),stop:0.50 rgba(56,150,143,0.92),stop:1 rgba(126,105,172,0.92)); border: 1px solid rgba(189,235,255,0.34); border-radius: 15px; padding: 7px; outline: none; selection-background-color: rgba(189,235,255,0.26); }
QWidget[uploadOptionPopupViewport="true"] { background: transparent; border-radius: 14px; }
QComboBox[uploadOptionCombo="true"] QAbstractItemView::item, QAbstractItemView[uploadOptionPopup="true"]::item { min-height: 28px; padding: 8px 14px; margin: 2px; border-radius: 9px; color: #f3fbff; background: rgba(189,235,255,0.10); }
QComboBox[uploadOptionCombo="true"] QAbstractItemView::item:hover, QComboBox[uploadOptionCombo="true"] QAbstractItemView::item:selected, QAbstractItemView[uploadOptionPopup="true"]::item:hover, QAbstractItemView[uploadOptionPopup="true"]::item:selected { background: qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 rgba(189,235,255,0.34),stop:1 rgba(216,199,255,0.28)); color: #ffffff; }
QGroupBox { background: rgba(18,27,38,0.60); border: 1px solid rgba(189,213,240,0.10); border-radius: 18px; padding: 24px 16px 16px; font-weight: 800; color: #edf5ff; }
QGroupBox::title { padding: 4px 14px; margin-left: 16px; background: rgba(255,255,255,0.08); border-radius: 9px; color: #dfeeff; }
QTabWidget::pane { background: rgba(18,27,38,0.42); border: 1px solid rgba(189,213,240,0.09); border-radius: 14px; }
QTabBar::tab { padding: 8px 20px; border-radius: 9px; color: #8192a7; font-weight: 700; }
QTabBar::tab:selected { background: rgba(255,255,255,0.08); color: #eff7ff; font-weight: 800; }
QListWidget { background: rgba(11,16,23,0.42); border: 1px solid rgba(189,213,240,0.10); border-radius: 16px; color: #cbd5e1; padding: 7px; outline: none; }
QListWidget[uploadQueueList="true"] { background: qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 rgba(141,198,255,0.16),stop:0.48 rgba(72,187,173,0.12),stop:1 rgba(216,199,255,0.13)); border: 1px solid rgba(156,199,255,0.24); border-radius: 18px; padding: 8px; }
QWidget[uploadQueueViewport="true"] { background: transparent; border-radius: 16px; }
QListWidget::item { padding: 11px 14px; margin: 3px; border-radius: 11px; color: #cbd5e1; }
QListWidget[uploadQueueList="true"]::item { background: rgba(156,199,255,0.08); border: 1px solid rgba(156,199,255,0.10); border-radius: 13px; margin: 4px 5px 4px 2px; }
QListWidget[uploadQueueList="true"]::item:hover { background: rgba(141,198,255,0.17); border-color: rgba(141,198,255,0.26); color: #f4fbff; }
QListWidget[uploadQueueList="true"]::item:selected { background: qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 rgba(141,198,255,0.24),stop:1 rgba(72,187,173,0.20)); border-color: rgba(189,235,255,0.34); color: #ffffff; }
QListWidget::item:hover { background: rgba(255,255,255,0.07); color: #eef7ff; }
QListWidget::item:selected { background: qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 rgba(141,198,255,0.17),stop:1 rgba(72,187,173,0.13)); color: #ffffff; }
QScrollBar:vertical { background: rgba(255,255,255,0.03); width: 6px; border-radius: 3px; }
QScrollBar::handle:vertical { background: rgba(255,255,255,0.12); border-radius: 3px; min-height: 40px; }
QScrollBar[uploadQueueScrollBar="true"]:vertical { background: rgba(141,198,255,0.13); border: 1px solid rgba(141,198,255,0.20); width: 13px; margin: 9px 3px 9px 3px; border-radius: 7px; }
QScrollBar[uploadQueueScrollBar="true"]::handle:vertical { background: qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 rgba(141,198,255,0.60),stop:0.48 rgba(72,187,173,0.52),stop:1 rgba(216,199,255,0.50)); border: 1px solid rgba(189,235,255,0.34); border-radius: 6px; min-height: 48px; }
QScrollBar[uploadQueueScrollBar="true"]::handle:vertical:hover { background: qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 rgba(170,215,255,0.74),stop:0.48 rgba(99,211,201,0.66),stop:1 rgba(228,214,255,0.62)); border-color: rgba(189,235,255,0.52); }
QScrollBar[uploadQueueScrollBar="true"]::add-line:vertical, QScrollBar[uploadQueueScrollBar="true"]::sub-line:vertical { height: 0px; background: transparent; border: none; }
QScrollBar[uploadQueueScrollBar="true"]::add-page:vertical, QScrollBar[uploadQueueScrollBar="true"]::sub-page:vertical { background: rgba(72,187,173,0.10); border-radius: 6px; }
QScrollBar:horizontal { background: rgba(255,255,255,0.03); height: 6px; border-radius: 3px; }
QScrollBar::handle:horizontal { background: rgba(255,255,255,0.12); border-radius: 3px; }
QLabel[galleryState="true"] { color: #a7b6c8; font-size: 15px; font-weight: 600; padding: 22px; }
QLabel[galleryMeta="true"] { color: #9fb0c5; font-size: 12px; font-weight: 700; padding: 0 4px; }
QToolButton[galleryThumb="true"] { background: rgba(255,255,255,0.08); border-radius: 14px; border: 1px solid rgba(189,213,240,0.13); padding: 4px; }
QToolButton[galleryThumb="true"]:hover { background: rgba(255,255,255,0.13); border-color: rgba(141,198,255,0.34); }
QToolButton[galleryThumb="true"][selected="true"] { background: rgba(141,198,255,0.13); border: 2px solid rgba(141,198,255,0.58); }
QToolButton[profileAction="true"] { background: qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 rgba(255,255,255,0.12),stop:1 rgba(156,199,255,0.07)); border: 1px solid rgba(148,163,184,0.16); border-radius: 14px; padding: 8px 12px; color: #dce7f5; font-weight: 800; text-align: left; }
QToolButton[profileAction="true"]:hover { background: qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 rgba(255,255,255,0.18),stop:1 rgba(156,199,255,0.13)); border-color: rgba(156,199,255,0.28); }
QProgressBar { min-height: 10px; max-height: 10px; border-radius: 5px; border: 1px solid rgba(255,255,255,0.08); background: rgba(255,255,255,0.06); }
QProgressBar::chunk { border-radius: 5px; background: qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #8DC6FF,stop:0.52 #48BBAD,stop:1 #D8C7FF); }
QDialog { background: #111a24; border: 1px solid rgba(189,213,240,0.13); border-radius: 18px; }
QDialog#profileDialog, QDialog#settingsDialog { background: #111a24; }
QDialog#imageInfoDialog { background: #111a24; }
QMessageBox { background: #111a24; border: 1px solid rgba(189,213,240,0.13); border-radius: 18px; }
QMessageBox#appleMessageBox { background: #111a24; border: 1px solid rgba(189,213,240,0.18); border-radius: 18px; }
QGroupBox[infoSection="true"] { background: rgba(18,27,38,0.68); border-color: rgba(189,213,240,0.13); }
QLabel[infoValue="true"] { color: #dbeafe; font-size: 13px; font-weight: 700; }
QLabel[infoLoading="true"] { color: #9fb1c6; background: rgba(255,255,255,0.05); border: 1px solid rgba(189,213,240,0.10); border-radius: 12px; padding: 12px; font-weight: 800; }
QTextEdit[infoUrlBox="true"], QTextEdit[infoDescBox="true"] { background: rgba(9,14,20,0.46); border: 1px solid rgba(189,213,240,0.13); border-radius: 12px; padding: 9px 10px; color: #dbeafe; font-weight: 650; selection-background-color: rgba(141,198,255,0.28); }
QTextEdit[infoUrlBox="true"] { color: #bfe3ff; }
QLabel[profileName="true"] { color: #f5f9ff; font-size: 18px; font-weight: 900; }
QLabel[profileMeta="true"], QLabel[profileHint="true"] { color: #9fb1c6; font-size: 13px; font-weight: 650; }
QLabel[profileStatus="ok"] { color: #79d8cc; background: rgba(72,187,173,0.12); border: 1px solid rgba(72,187,173,0.20); border-radius: 10px; padding: 7px 10px; font-weight: 800; }
QLabel[profileStatus="idle"] { color: #9fb1c6; background: rgba(255,255,255,0.05); border: 1px solid rgba(189,213,240,0.10); border-radius: 10px; padding: 7px 10px; font-weight: 700; }
QGroupBox[profileSection="true"] { background: rgba(18,27,38,0.68); border-color: rgba(189,213,240,0.13); }
QLabel[settingsLabel="true"] { color: #dbeafe; font-size: 13px; font-weight: 850; }
QPushButton[stepperBtn="true"] { font-size: 18px; font-weight: 800; border-radius: 9px; padding: 0; min-height: 30px; }
QToolTip { background: rgba(30,36,48,0.96); color: #e2e8f0; border: 1px solid rgba(255,255,255,0.1); border-radius: 8px; padding: 6px 12px; }
QStatusBar { background: rgba(10,15,20,0.62); border-top: 1px solid rgba(189,213,240,0.08); color: #7f91a7; padding: 4px 10px; }
QLabel[statusText="true"] { color: #8ea0b5; font-size: 12px; }
QLabel[statusPill="checking"] { color: #FBBF24; background: rgba(251,191,36,0.10); border: 1px solid rgba(251,191,36,0.16); border-radius: 9px; padding: 3px 9px; font-weight: 700; }
QLabel[statusPill="ok"] { color: #79d8cc; background: rgba(72,187,173,0.12); border: 1px solid rgba(72,187,173,0.22); border-radius: 9px; padding: 3px 9px; font-weight: 700; }
QLabel[statusPill="error"] { color: #FB7185; background: rgba(251,113,133,0.12); border: 1px solid rgba(251,113,133,0.20); border-radius: 9px; padding: 3px 9px; font-weight: 700; }
        )").arg("Noto Sans SC, WenQuanYi Micro Hei, Sans Serif");
    }

    // Light themes mirror XMail's translucent white panels and soft theme-color gradients.
    return QString(R"(
* { font-family: "Noto Sans SC","WenQuanYi Micro Hei","Sans Serif"; font-size: 13px; color: %1; }
QMainWindow { background: qradialgradient(cx:0.22,cy:0.10,radius:1.10,fx:0.22,fy:0.10,stop:0 rgba(255,255,255,0.98),stop:0.24 %2,stop:0.48 rgba(248,252,252,0.94),stop:0.72 %3,stop:0.90 %4,stop:1 rgba(255,255,255,0.98)); }
QWidget#androidShell { background: transparent; }
QStackedWidget#contentStack { background: qradialgradient(cx:0.42,cy:0.16,radius:0.98,fx:0.42,fy:0.16,stop:0 rgba(255,255,255,0.96),stop:0.52 %5,stop:1 rgba(255,255,255,0.72)); border: 1px solid rgba(255,255,255,0.82); border-radius: 22px; }
QFrame#sideNavigation { background: qradialgradient(cx:0.50,cy:0.08,radius:0.94,fx:0.50,fy:0.08,stop:0 rgba(255,255,255,0.96),stop:0.56 %6,stop:1 rgba(255,255,255,0.74)); border: 1px solid rgba(255,255,255,0.82); border-radius: 22px; }
QToolButton[sideNavButton="true"] { background: transparent; border: 1px solid transparent; border-radius: 16px; padding: 8px 6px; color: %7; font-weight: 800; }
QToolButton[sideNavButton="true"]:hover { background: rgba(255,255,255,0.66); border-color: rgba(255,255,255,0.82); color: %8; }
QToolButton[sideNavButton="true"]:checked { background: qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 rgba(255,255,255,0.94),stop:0.48 %9,stop:1 %2); border-color: rgba(255,255,255,0.96); color: %8; }
QMenuBar { background: %6; border-bottom: 1px solid rgba(255,255,255,0.70); padding: 5px 10px; font-weight: 600; }
QMenuBar::item { padding: 7px 15px; margin: 1px 2px; border-radius: 9px; color: %7; }
QMenuBar::item:selected { background: %9; color: %8; }
QMenu { background: rgba(255,255,255,0.96); border: 1px solid rgba(255,255,255,0.84); border-radius: 14px; padding: 8px; }
QMenu::item { padding: 9px 34px 9px 16px; margin: 2px; border-radius: 9px; color: %7; }
QMenu::item:selected { background: %9; color: %8; }
QToolBar { background: rgba(255,255,255,0.72); border-bottom: 1px solid rgba(255,255,255,0.70); padding: 8px 10px; spacing: 8px; }
QToolBar QToolButton { padding: 7px 12px; border-radius: 10px; color: %8; font-weight: 700; border: 1px solid rgba(255,255,255,0.70); background: rgba(255,255,255,0.82); }
QToolBar QToolButton:hover { background: %9; color: %1; border-color: rgba(255,255,255,0.92); }
QPushButton { padding: 9px 20px; border-radius: 10px; font-weight: 700; border: 1px solid rgba(255,255,255,0.72); min-height: 22px; background: rgba(255,255,255,0.68); color: %7; }
QPushButton:hover { background: rgba(255,255,255,0.86); border-color: rgba(255,255,255,0.92); color: %8; }
QPushButton:disabled { color: rgba(100,116,139,0.45); background: rgba(255,255,255,0.36); border-color: rgba(255,255,255,0.44); }
QPushButton#primaryBtn, QPushButton[primaryBtn="true"] { background: qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 rgba(255,255,255,0.96),stop:0.42 %9,stop:1 %2); color: %8; font-weight: 800; border: 1px solid rgba(255,255,255,0.88); }
QPushButton#primaryBtn:hover, QPushButton[primaryBtn="true"]:hover { background: qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 rgba(255,255,255,1.00),stop:0.42 %9,stop:1 %3); color: %8; border-color: rgba(255,255,255,0.98); }
QPushButton[flat="true"] { background: rgba(255,255,255,0.64); border: 1px solid rgba(229,231,235,0.80); color: %7; }
QPushButton[flat="true"]:hover { background: rgba(255,255,255,0.84); border-color: rgba(255,255,255,0.92); color: %8; }
QLineEdit, QComboBox, QSpinBox { padding: 9px 14px; border-radius: 10px; border: 1px solid rgba(255,255,255,0.70); background: qradialgradient(cx:0.50,cy:0.18,radius:0.82,fx:0.50,fy:0.18,stop:0 rgba(255,255,255,0.96),stop:1 rgba(255,255,255,0.76)); color: %1; }
QLineEdit:focus, QComboBox:focus, QSpinBox:focus { border-color: %8; }
QComboBox::drop-down { border: none; width: 24px; subcontrol-origin: padding; subcontrol-position: right center; }
QComboBox::down-arrow { width: 10px; height: 10px; }
QComboBox QAbstractItemView { background: rgba(255,255,255,0.96); border: 1px solid rgba(0,0,0,0.08); border-radius: 8px; padding: 4px; }
QComboBox QAbstractItemView::item { padding: 8px 14px; border-radius: 6px; color: %1; }
QComboBox QAbstractItemView::item:selected { background: %9; color: %8; }
QComboBox[uploadOptionCombo="true"] { min-height: 24px; padding: 9px 38px 9px 14px; border-radius: 14px; border: 1px solid rgba(255,255,255,0.88); background: qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 rgba(255,255,255,0.94),stop:0.46 %9,stop:1 %2); color: %8; font-weight: 850; }
QComboBox[uploadOptionCombo="true"]:hover { border-color: rgba(255,255,255,0.98); background: qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 rgba(255,255,255,1.00),stop:0.46 %9,stop:1 %3); }
QComboBox[uploadOptionCombo="true"]:focus { border-color: %8; background: qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 rgba(255,255,255,1.00),stop:0.48 %9,stop:1 %2); }
QComboBox[uploadOptionCombo="true"]::drop-down { width: 34px; border: none; border-left: 1px solid rgba(255,255,255,0.64); border-top-right-radius: 14px; border-bottom-right-radius: 14px; background: rgba(255,255,255,0.34); }
QComboBox[uploadOptionCombo="true"]::down-arrow { image: none; width: 0; height: 0; border-left: 5px solid transparent; border-right: 5px solid transparent; border-top: 6px solid %8; margin-right: 12px; }
QComboBox[uploadOptionCombo="true"] QAbstractItemView, QAbstractItemView[uploadOptionPopup="true"] { background: qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 %9,stop:0.50 %2,stop:1 %5); border: 1px solid rgba(255,255,255,0.90); border-radius: 15px; padding: 7px; outline: none; selection-background-color: %9; }
QWidget[uploadOptionPopupViewport="true"] { background: transparent; border-radius: 14px; }
QComboBox[uploadOptionCombo="true"] QAbstractItemView::item, QAbstractItemView[uploadOptionPopup="true"]::item { min-height: 28px; padding: 8px 14px; margin: 2px; border-radius: 9px; color: %1; background: rgba(255,255,255,0.42); }
QComboBox[uploadOptionCombo="true"] QAbstractItemView::item:hover, QComboBox[uploadOptionCombo="true"] QAbstractItemView::item:selected, QAbstractItemView[uploadOptionPopup="true"]::item:hover, QAbstractItemView[uploadOptionPopup="true"]::item:selected { background: qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 rgba(255,255,255,0.74),stop:1 %9); color: %8; }
QGroupBox { background: qradialgradient(cx:0.45,cy:0.12,radius:0.96,fx:0.45,fy:0.12,stop:0 rgba(255,255,255,0.98),stop:0.52 %5,stop:1 rgba(255,255,255,0.78)); border: 1px solid rgba(255,255,255,0.78); border-radius: 18px; padding: 24px 16px 16px; font-weight: 800; color: %1; }
QGroupBox::title { padding: 4px 14px; margin-left: 16px; background: rgba(255,255,255,0.74); border-radius: 9px; }
QTabWidget::pane { background: qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 rgba(255,255,255,0.66),stop:1 rgba(255,255,255,0.40)); border: 1px solid rgba(255,255,255,0.68); border-radius: 14px; }
QTabBar::tab { padding: 8px 20px; border-radius: 9px; color: rgba(125,147,153,0.90); font-weight: 700; }
QTabBar::tab:selected { background: rgba(255,255,255,0.78); color: %8; font-weight: 800; }
QListWidget { background: rgba(255,255,255,0.76); border: 1px solid rgba(255,255,255,0.76); border-radius: 16px; color: %7; padding: 7px; outline: none; }
QListWidget[uploadQueueList="true"] { background: qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 %9,stop:0.52 %2,stop:1 %5); border: 1px solid rgba(255,255,255,0.84); border-radius: 18px; padding: 8px; }
QWidget[uploadQueueViewport="true"] { background: transparent; border-radius: 16px; }
QListWidget::item { padding: 11px 14px; margin: 3px; border-radius: 11px; }
QListWidget[uploadQueueList="true"]::item { background: rgba(255,255,255,0.46); border: 1px solid rgba(255,255,255,0.58); border-radius: 13px; margin: 4px 5px 4px 2px; }
QListWidget[uploadQueueList="true"]::item:hover { background: %9; border-color: rgba(255,255,255,0.82); color: %8; }
QListWidget[uploadQueueList="true"]::item:selected { background: qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 %9,stop:1 %2); border-color: rgba(255,255,255,0.92); color: %8; }
QListWidget::item:hover { background: rgba(255,255,255,0.72); color: %8; }
QListWidget::item:selected { background: qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 %9,stop:1 rgba(255,255,255,0.88)); color: %8; }
QLabel[galleryState="true"] { color: rgba(125,147,153,0.92); font-size: 15px; font-weight: 600; padding: 22px; }
QLabel[galleryMeta="true"] { color: rgba(125,147,153,0.92); font-size: 12px; font-weight: 700; padding: 0 4px; }
QToolButton[galleryThumb="true"] { background: rgba(255,255,255,0.62); border-radius: 14px; border: 1px solid rgba(255,255,255,0.78); padding: 4px; }
QToolButton[galleryThumb="true"]:hover { background: rgba(255,255,255,0.86); border-color: rgba(255,255,255,0.98); }
QToolButton[galleryThumb="true"][selected="true"] { background: rgba(255,255,255,0.78); border: 2px solid %8; }
QToolButton[profileAction="true"] { background: qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 rgba(255,255,255,0.84),stop:1 rgba(255,255,255,0.58)); border: 1px solid rgba(255,255,255,0.76); border-radius: 14px; padding: 8px 12px; color: %7; font-weight: 800; text-align: left; }
QToolButton[profileAction="true"]:hover { background: qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 rgba(255,255,255,0.96),stop:1 %9); border-color: rgba(255,255,255,0.96); color: %8; }
QScrollBar:vertical { background: rgba(255,255,255,0.42); width: 6px; border-radius: 3px; }
QScrollBar::handle:vertical { background: rgba(31,159,147,0.22); border-radius: 3px; min-height: 40px; }
QScrollBar[uploadQueueScrollBar="true"]:vertical { background: %2; border: 1px solid rgba(255,255,255,0.78); width: 13px; margin: 9px 3px 9px 3px; border-radius: 7px; }
QScrollBar[uploadQueueScrollBar="true"]::handle:vertical { background: qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 %9,stop:0.48 %2,stop:1 %3); border: 1px solid rgba(255,255,255,0.88); border-radius: 6px; min-height: 48px; }
QScrollBar[uploadQueueScrollBar="true"]::handle:vertical:hover { background: qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 %2,stop:0.48 %9,stop:1 %3); border-color: rgba(255,255,255,0.96); }
QScrollBar[uploadQueueScrollBar="true"]::add-line:vertical, QScrollBar[uploadQueueScrollBar="true"]::sub-line:vertical { height: 0px; background: transparent; border: none; }
QScrollBar[uploadQueueScrollBar="true"]::add-page:vertical, QScrollBar[uploadQueueScrollBar="true"]::sub-page:vertical { background: %9; border-radius: 6px; }
QScrollBar:horizontal { background: rgba(255,255,255,0.42); height: 6px; border-radius: 3px; }
QScrollBar::handle:horizontal { background: rgba(31,159,147,0.22); border-radius: 3px; }
QProgressBar { min-height: 10px; max-height: 10px; border-radius: 5px; border: 1px solid rgba(255,255,255,0.72); background: rgba(255,255,255,0.42); }
QProgressBar::chunk { border-radius: 5px; background: qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 %8,stop:0.34 %2,stop:0.68 #8DC6FF,stop:1 %3); }
QDialog { background: rgba(247,252,252,0.98); border: 1px solid rgba(255,255,255,0.88); border-radius: 18px; }
QDialog#profileDialog, QDialog#settingsDialog { background: rgba(247,252,252,0.98); }
QDialog#imageInfoDialog { background: rgba(247,252,252,0.98); }
QMessageBox { background: rgba(247,252,252,0.98); border: 1px solid rgba(255,255,255,0.88); border-radius: 18px; }
QMessageBox#appleMessageBox { background: rgba(247,252,252,1.00); border: 1px solid rgba(255,255,255,0.92); border-radius: 18px; }
QGroupBox[infoSection="true"] { background: rgba(255,255,255,0.78); border-color: rgba(255,255,255,0.84); }
QLabel[infoValue="true"] { color: %1; font-size: 13px; font-weight: 700; }
QLabel[infoLoading="true"] { color: %7; background: rgba(255,255,255,0.60); border: 1px solid rgba(255,255,255,0.76); border-radius: 12px; padding: 12px; font-weight: 800; }
QTextEdit[infoUrlBox="true"], QTextEdit[infoDescBox="true"] { background: rgba(255,255,255,0.68); border: 1px solid rgba(255,255,255,0.78); border-radius: 12px; padding: 9px 10px; color: %1; font-weight: 650; selection-background-color: %9; }
QTextEdit[infoUrlBox="true"] { color: %8; }
QLabel[profileName="true"] { color: %1; font-size: 18px; font-weight: 900; }
QLabel[profileMeta="true"], QLabel[profileHint="true"] { color: %7; font-size: 13px; font-weight: 650; }
QLabel[profileStatus="ok"] { color: %8; background: %9; border: 1px solid rgba(255,255,255,0.80); border-radius: 10px; padding: 7px 10px; font-weight: 800; }
QLabel[profileStatus="idle"] { color: %7; background: rgba(255,255,255,0.60); border: 1px solid rgba(255,255,255,0.76); border-radius: 10px; padding: 7px 10px; font-weight: 700; }
QGroupBox[profileSection="true"] { background: rgba(255,255,255,0.78); border-color: rgba(255,255,255,0.84); }
QLabel[settingsLabel="true"] { color: %1; font-size: 13px; font-weight: 850; }
QPushButton[stepperBtn="true"] { font-size: 18px; font-weight: 800; border-radius: 9px; padding: 0; min-height: 30px; }
QToolTip { background: rgba(255,255,255,0.95); color: %1; border: 1px solid rgba(0,0,0,0.08); border-radius: 8px; padding: 6px 12px; }
QStatusBar { background: rgba(255,255,255,0.76); border-top: 1px solid rgba(255,255,255,0.70); color: rgba(125,147,153,0.92); padding: 4px 10px; }
QLabel[statusText="true"] { color: rgba(125,147,153,0.92); font-size: 12px; }
QLabel[statusPill="checking"] { color: #B45309; background: rgba(245,158,11,0.12); border: 1px solid rgba(245,158,11,0.18); border-radius: 9px; padding: 3px 9px; font-weight: 700; }
QLabel[statusPill="ok"] { color: %8; background: %9; border: 1px solid rgba(255,255,255,0.72); border-radius: 9px; padding: 3px 9px; font-weight: 700; }
QLabel[statusPill="error"] { color: #E11D48; background: rgba(251,113,133,0.13); border: 1px solid rgba(251,113,133,0.22); border-radius: 9px; padding: 3px 9px; font-weight: 700; }
    )")
           .arg(c.textPrimary, c.accentSoft, c.bgGradientStart, c.bgGradientEnd,
               c.cardBg, c.menubarBg, c.textSecondary, c.accent, c.accentLight);
}

