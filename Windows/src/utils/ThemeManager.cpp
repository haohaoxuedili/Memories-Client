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
    // ---- Mint (default green) ----
    m_themes["mint"] = {
        "薄荷绿", "#f0faf5", "#e8f5ed",
        "rgba(255,255,255,0.55)", "rgba(255,255,255,0.5)", "16px",
        "#0f172a", "#334155", "#94a3b8",
        "#1D6E5A", "#175A48", "rgba(29,110,90,0.10)", "#38BDF8", "#F59E0B",
        "rgba(255,255,255,0.64)", "rgba(255,255,255,0.72)",
        "rgba(255,255,255,0.6)", "rgba(255,255,255,0.65)",
        "rgba(30,41,59,0.15)", "rgba(30,41,59,0.18)",
        "#fdfcfb", "#e8e3de"
    };

    // ---- Rose ----
    m_themes["rose"] = {
        "玫瑰粉", "#fef5f7", "#fce4ec",
        "rgba(255,255,255,0.55)", "rgba(255,240,242,0.5)", "16px",
        "#2d1b1e", "#5c3b3f", "#b08d91",
        "#C44569", "#a83655", "rgba(196,69,105,0.10)", "#60A5FA", "#F97316",
        "rgba(255,255,255,0.64)", "rgba(255,255,255,0.72)",
        "rgba(255,255,255,0.6)", "rgba(255,255,255,0.65)",
        "rgba(45,27,30,0.12)", "rgba(45,27,30,0.15)",
        "#fdfcfb", "#f0e4e6"
    };

    // ---- Sky ----
    m_themes["sky"] = {
        "天空蓝", "#f0f7ff", "#e3f0fd",
        "rgba(255,255,255,0.55)", "rgba(240,246,255,0.5)", "16px",
        "#0f172a", "#1e3a5f", "#7b93b0",
        "#2563EB", "#1d4ed8", "rgba(37,99,235,0.10)", "#14B8A6", "#F59E0B",
        "rgba(255,255,255,0.64)", "rgba(255,255,255,0.72)",
        "rgba(255,255,255,0.6)", "rgba(255,255,255,0.65)",
        "rgba(15,23,42,0.12)", "rgba(15,23,42,0.15)",
        "#f8fafd", "#e8eef8"
    };

    // ---- Lavender ----
    m_themes["lavender"] = {
        "薰衣草紫", "#f8f6ff", "#ede9fe",
        "rgba(255,255,255,0.55)", "rgba(245,243,255,0.5)", "16px",
        "#1e1b4b", "#3b3670", "#9d99c7",
        "#7C3AED", "#6d28d9", "rgba(124,58,237,0.10)", "#06B6D4", "#F43F5E",
        "rgba(255,255,255,0.64)", "rgba(255,255,255,0.72)",
        "rgba(255,255,255,0.6)", "rgba(255,255,255,0.65)",
        "rgba(30,27,75,0.12)", "rgba(30,27,75,0.15)",
        "#fdfbff", "#edeaf5"
    };

    // ---- Sunset ----
    m_themes["sunset"] = {
        "日落橙", "#fffaf5", "#fff3e4",
        "rgba(255,255,255,0.55)", "rgba(255,248,240,0.5)", "16px",
        "#2d1a0f", "#5c3a22", "#b09078",
        "#EA580C", "#c2410a", "rgba(234,88,12,0.10)", "#0EA5E9", "#EAB308",
        "rgba(255,255,255,0.64)", "rgba(255,255,255,0.72)",
        "rgba(255,255,255,0.6)", "rgba(255,255,255,0.65)",
        "rgba(45,26,15,0.12)", "rgba(45,26,15,0.15)",
        "#fdfaf7", "#f0e8dd"
    };

    // ---- Ocean (deep teal) ----
    m_themes["ocean"] = {
        "深海蓝", "#f2f9fa", "#e4f3f5",
        "rgba(255,255,255,0.55)", "rgba(235,248,250,0.5)", "16px",
        "#0f282d", "#1e4d56", "#6b99a2",
        "#0D9488", "#0f766e", "rgba(13,148,136,0.10)", "#2563EB", "#F97316",
        "rgba(255,255,255,0.64)", "rgba(255,255,255,0.72)",
        "rgba(255,255,255,0.6)", "rgba(255,255,255,0.65)",
        "rgba(15,40,45,0.12)", "rgba(15,40,45,0.15)",
        "#f8fcfd", "#e4eff2"
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
* { font-family: "%1"; font-size: 13px; color: #e2e8f0; }
QMainWindow { background: qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 #121a26,stop:0.28 #152f31,stop:0.56 #262235,stop:0.78 #2b211b,stop:1 #101418); }
QMenuBar { background: rgba(20,24,33,0.88); border-bottom: 1px solid rgba(255,255,255,0.06); padding: 5px 10px; }
QMenuBar::item { padding: 7px 15px; margin: 1px 2px; border-radius: 9px; color: #94a3b8; font-weight: 600; }
QMenuBar::item:selected { background: rgba(83,196,158,0.12); color: #53C49E; }
QMenu { background: rgba(20,24,33,0.96); border: 1px solid rgba(255,255,255,0.08); border-radius: 14px; padding: 8px; }
QMenu::item { padding: 9px 34px 9px 16px; margin: 2px; border-radius: 9px; color: #94a3b8; }
QMenu::item:selected { background: rgba(83,196,158,0.12); color: #53C49E; }
QToolBar { background: rgba(20,24,33,0.75); border-bottom: 1px solid rgba(255,255,255,0.06); padding: 7px 10px; spacing: 8px; }
QToolBar QToolButton { padding: 7px 14px; border-radius: 10px; color: #cbd5e1; font-weight: 700; border: 1px solid rgba(255,255,255,0.06); background: rgba(255,255,255,0.035); }
QToolBar QToolButton:hover { background: rgba(255,255,255,0.09); color: #53C49E; border-color: rgba(83,196,158,0.24); }
QPushButton { padding: 9px 20px; border-radius: 10px; font-weight: 700; border: 1px solid transparent; min-height: 22px; }
QPushButton:disabled { color: rgba(203,213,225,0.38); background: rgba(255,255,255,0.035); border-color: rgba(255,255,255,0.04); }
QPushButton#primaryBtn, QPushButton[primaryBtn="true"] { background: qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 #53C49E,stop:0.58 #38BDF8,stop:1 #F59E0B); color: #0f172a; font-weight: 800; border: 1px solid rgba(255,255,255,0.16); }
QPushButton#primaryBtn:hover, QPushButton[primaryBtn="true"]:hover { background: qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 #6BD5AE,stop:0.55 #60A5FA,stop:1 #FDBA74); }
QPushButton[flat="true"] { background: rgba(255,255,255,0.05); border: 1px solid rgba(255,255,255,0.06); color: #94a3b8; }
QPushButton[flat="true"]:hover { background: rgba(255,255,255,0.08); border-color: rgba(83,196,158,0.18); }
QLineEdit, QComboBox, QSpinBox { padding: 9px 14px; border-radius: 10px; border: 1px solid rgba(255,255,255,0.08); background: rgba(20,24,33,0.7); color: #e2e8f0; }
QLineEdit:focus, QComboBox:focus, QSpinBox:focus { border-color: #53C49E; }
QComboBox::drop-down { border: none; width: 24px; subcontrol-origin: padding; subcontrol-position: right center; }
QComboBox::down-arrow { width: 10px; height: 10px; }
QComboBox QAbstractItemView { background: rgba(20,24,33,0.97); border: 1px solid rgba(255,255,255,0.08); border-radius: 8px; color: #e2e8f0; padding: 4px; }
QComboBox QAbstractItemView::item { padding: 8px 14px; border-radius: 6px; }
QComboBox QAbstractItemView::item:selected { background: rgba(83,196,158,0.15); color: #53C49E; }
QGroupBox { background: rgba(20,24,33,0.5); border: 1px solid rgba(255,255,255,0.06); border-radius: 16px; padding: 24px 16px 16px; font-weight: 700; color: #e2e8f0; }
QGroupBox::title { padding: 4px 14px; margin-left: 16px; background: rgba(30,36,48,0.8); border-radius: 8px; }
QTabWidget::pane { background: rgba(20,24,33,0.4); border: 1px solid rgba(255,255,255,0.05); border-radius: 12px; }
QTabBar::tab { padding: 8px 20px; border-radius: 8px; color: #64748b; font-weight: 500; }
QTabBar::tab:selected { background: rgba(255,255,255,0.06); color: #53C49E; font-weight: 700; }
QListWidget { background: rgba(20,24,33,0.46); border: 1px solid rgba(255,255,255,0.07); border-radius: 14px; color: #cbd5e1; padding: 6px; outline: none; }
QListWidget::item { padding: 11px 14px; margin: 2px; border-radius: 10px; color: #cbd5e1; }
QListWidget::item:hover { background: rgba(255,255,255,0.06); }
QListWidget::item:selected { background: rgba(83,196,158,0.14); color: #e9fff7; }
QScrollBar:vertical { background: rgba(255,255,255,0.03); width: 6px; border-radius: 3px; }
QScrollBar::handle:vertical { background: rgba(255,255,255,0.12); border-radius: 3px; min-height: 40px; }
QScrollBar:horizontal { background: rgba(255,255,255,0.03); height: 6px; border-radius: 3px; }
QScrollBar::handle:horizontal { background: rgba(255,255,255,0.12); border-radius: 3px; }
QLabel[galleryState="true"] { color: #a7b6c8; font-size: 15px; font-weight: 600; padding: 22px; }
QLabel[galleryMeta="true"] { color: #9fb0c5; font-size: 12px; font-weight: 700; padding: 0 4px; }
QToolButton[galleryThumb="true"] { background: rgba(255,255,255,0.10); border-radius: 12px; border: 1px solid rgba(255,255,255,0.12); padding: 4px; }
QToolButton[galleryThumb="true"]:hover { background: rgba(255,255,255,0.16); border-color: rgba(83,196,158,0.42); }
QToolButton[galleryThumb="true"][selected="true"] { background: rgba(255,255,255,0.14); border: 2px solid #53C49E; }
QProgressBar { min-height: 10px; max-height: 10px; border-radius: 5px; border: 1px solid rgba(255,255,255,0.08); background: rgba(255,255,255,0.06); }
QProgressBar::chunk { border-radius: 5px; background: qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #53C49E,stop:0.42 #38BDF8,stop:0.72 #FB7185,stop:1 #F59E0B); }
QDialog { background: qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 #1e2433,stop:0.45 #172c30,stop:1 #141820); border-radius: 16px; }
QToolTip { background: rgba(30,36,48,0.96); color: #e2e8f0; border: 1px solid rgba(255,255,255,0.1); border-radius: 8px; padding: 6px 12px; }
QStatusBar { background: rgba(20,24,33,0.72); border-top: 1px solid rgba(255,255,255,0.06); color: #64748b; padding: 4px 10px; }
QLabel[statusText="true"] { color: #8ea0b5; font-size: 12px; }
QLabel[statusPill="checking"] { color: #FBBF24; background: rgba(251,191,36,0.10); border: 1px solid rgba(251,191,36,0.16); border-radius: 9px; padding: 3px 9px; font-weight: 700; }
QLabel[statusPill="ok"] { color: #53C49E; background: rgba(83,196,158,0.12); border: 1px solid rgba(83,196,158,0.20); border-radius: 9px; padding: 3px 9px; font-weight: 700; }
QLabel[statusPill="error"] { color: #FB7185; background: rgba(251,113,133,0.12); border: 1px solid rgba(251,113,133,0.20); border-radius: 9px; padding: 3px 9px; font-weight: 700; }
        )").arg("Noto Sans SC, WenQuanYi Micro Hei, Sans Serif");
    }

    // Light themes - build from color palette
    return QString(R"(
* { font-family: "Noto Sans SC","WenQuanYi Micro Hei","Sans Serif"; font-size: 13px; color: %1; }
QMainWindow { background: qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 #fff8f6,stop:0.26 #eef7ff,stop:0.52 #f4fff8,stop:0.76 #fff4e6,stop:1 #fff7fb); }
QMenuBar { background: qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 rgba(255,255,255,0.80),stop:0.5 rgba(255,255,255,0.56),stop:1 rgba(255,255,255,0.74)); border-bottom: 1px solid rgba(255,255,255,0.78); padding: 5px 10px; font-weight: 600; }
QMenuBar::item { padding: 7px 15px; margin: 1px 2px; border-radius: 9px; color: %4; }
QMenuBar::item:selected { background: qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 %5,stop:1 rgba(255,255,255,0.7)); color: %6; }
QMenu { background: rgba(255,255,255,0.94); border: 1px solid rgba(255,255,255,0.82); border-radius: 14px; padding: 8px; }
QMenu::item { padding: 9px 34px 9px 16px; margin: 2px; border-radius: 9px; color: %4; }
QMenu::item:selected { background: qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 %5,stop:1 rgba(255,255,255,0.64)); color: %6; }
QToolBar { background: qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 rgba(255,255,255,0.70),stop:0.5 rgba(255,255,255,0.45),stop:1 %7); border-bottom: 1px solid rgba(255,255,255,0.66); padding: 7px 10px; spacing: 8px; }
QToolBar QToolButton { padding: 7px 14px; border-radius: 10px; color: %4; font-weight: 700; border: 1px solid rgba(255,255,255,0.66); background: rgba(255,255,255,0.38); }
QToolBar QToolButton:hover { background: qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 rgba(255,255,255,0.90),stop:1 %5); color: %6; border-color: rgba(255,255,255,0.95); }
QPushButton { padding: 9px 20px; border-radius: 10px; font-weight: 700; border: 1px solid transparent; min-height: 22px; }
QPushButton:disabled { color: rgba(100,116,139,0.45); background: rgba(255,255,255,0.36); border-color: rgba(255,255,255,0.44); }
QPushButton#primaryBtn, QPushButton[primaryBtn="true"] { background: qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 %6,stop:0.55 %19,stop:1 %20); color: white; font-weight: 700; border: 1px solid rgba(255,255,255,0.55); }
QPushButton#primaryBtn:hover, QPushButton[primaryBtn="true"]:hover { background: qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 %8,stop:0.58 %19,stop:1 %20); }
QPushButton[flat="true"] { background: rgba(255,255,255,0.46); border: 1px solid rgba(255,255,255,0.58); color: %4; }
QPushButton[flat="true"]:hover { background: qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 rgba(255,255,255,0.82),stop:1 %5); color: %6; }
QLineEdit, QComboBox, QSpinBox { padding: 9px 14px; border-radius: 10px; border: 1px solid %9; background: qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 rgba(255,255,255,0.78),stop:1 %10); color: %1; }
QLineEdit:focus, QComboBox:focus, QSpinBox:focus { border-color: %6; }
QComboBox::drop-down { border: none; width: 24px; subcontrol-origin: padding; subcontrol-position: right center; }
QComboBox::down-arrow { width: 10px; height: 10px; }
QComboBox QAbstractItemView { background: rgba(255,255,255,0.96); border: 1px solid rgba(0,0,0,0.08); border-radius: 8px; padding: 4px; }
QComboBox QAbstractItemView::item { padding: 8px 14px; border-radius: 6px; color: %1; }
QComboBox QAbstractItemView::item:selected { background: %5; color: %6; }
QGroupBox { background: qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 rgba(255,255,255,0.70),stop:0.52 %11,stop:1 rgba(255,255,255,0.42)); border: 1px solid %12; border-radius: 16px; padding: 24px 16px 16px; font-weight: 700; color: %1; }
QGroupBox::title { padding: 4px 14px; margin-left: 16px; background: rgba(255,255,255,0.72); border-radius: 8px; }
QTabWidget::pane { background: qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 rgba(255,255,255,0.58),stop:1 rgba(255,255,255,0.34)); border: 1px solid rgba(255,255,255,0.55); border-radius: 12px; }
QTabBar::tab { padding: 8px 20px; border-radius: 8px; color: %13; font-weight: 500; }
QTabBar::tab:selected { background: rgba(255,255,255,0.7); color: %6; font-weight: 700; }
QListWidget { background: rgba(255,255,255,0.50); border: 1px solid rgba(255,255,255,0.62); border-radius: 14px; color: %4; padding: 6px; outline: none; }
QListWidget::item { padding: 11px 14px; margin: 2px; border-radius: 10px; }
QListWidget::item:hover { background: rgba(255,255,255,0.58); }
QListWidget::item:selected { background: qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 %5,stop:1 rgba(255,255,255,0.68)); color: %6; }
QLabel[galleryState="true"] { color: %13; font-size: 15px; font-weight: 600; padding: 22px; }
QLabel[galleryMeta="true"] { color: %13; font-size: 12px; font-weight: 700; padding: 0 4px; }
QToolButton[galleryThumb="true"] { background: rgba(255,255,255,0.58); border-radius: 12px; border: 1px solid rgba(255,255,255,0.78); padding: 4px; }
QToolButton[galleryThumb="true"]:hover { background: rgba(255,255,255,0.78); border-color: rgba(255,255,255,0.96); }
QToolButton[galleryThumb="true"][selected="true"] { background: rgba(255,255,255,0.72); border: 2px solid %6; }
QScrollBar:vertical { background: rgba(0,0,0,0.03); width: 6px; border-radius: 3px; }
QScrollBar::handle:vertical { background: %14; border-radius: 3px; min-height: 40px; }
QScrollBar:horizontal { background: rgba(0,0,0,0.03); height: 6px; border-radius: 3px; }
QScrollBar::handle:horizontal { background: %14; border-radius: 3px; }
QProgressBar { min-height: 10px; max-height: 10px; border-radius: 5px; border: 1px solid rgba(255,255,255,0.72); background: rgba(255,255,255,0.42); }
QProgressBar::chunk { border-radius: 5px; background: qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 %6,stop:0.34 %19,stop:0.68 #FB7185,stop:1 %20); }
QDialog { background: qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 %15,stop:0.48 rgba(255,255,255,0.88),stop:1 %16); border-radius: 16px; }
QToolTip { background: rgba(255,255,255,0.95); color: %1; border: 1px solid rgba(0,0,0,0.08); border-radius: 8px; padding: 6px 12px; }
QStatusBar { background: qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 %17,stop:1 rgba(255,255,255,0.50)); border-top: 1px solid rgba(255,255,255,0.62); color: %13; padding: 4px 10px; }
QLabel[statusText="true"] { color: %13; font-size: 12px; }
QLabel[statusPill="checking"] { color: #B45309; background: rgba(245,158,11,0.12); border: 1px solid rgba(245,158,11,0.18); border-radius: 9px; padding: 3px 9px; font-weight: 700; }
QLabel[statusPill="ok"] { color: %6; background: %5; border: 1px solid rgba(255,255,255,0.72); border-radius: 9px; padding: 3px 9px; font-weight: 700; }
QLabel[statusPill="error"] { color: #E11D48; background: rgba(251,113,133,0.13); border: 1px solid rgba(251,113,133,0.22); border-radius: 9px; padding: 3px 9px; font-weight: 700; }
    )")
        .arg(c.textPrimary, c.bgGradientStart, c.bgGradientEnd,
             c.textSecondary, c.accentLight, c.accent, c.toolbarBg,
             c.accentHover, c.inputBorder, c.inputBg,
             c.cardBg, c.cardBorder, c.textMuted,
             c.scrollbarHandle, c.dialogBgStart, c.dialogBgEnd, c.statusBg,
             c.menubarBg, c.accentSoft, c.accentWarm);
}

