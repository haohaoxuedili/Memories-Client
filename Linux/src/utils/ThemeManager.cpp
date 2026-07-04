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
        "#1D6E5A", "#175A48", "rgba(29,110,90,0.10)",
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
        "#C44569", "#a83655", "rgba(196,69,105,0.10)",
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
        "#2563EB", "#1d4ed8", "rgba(37,99,235,0.10)",
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
        "#7C3AED", "#6d28d9", "rgba(124,58,237,0.10)",
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
        "#EA580C", "#c2410a", "rgba(234,88,12,0.10)",
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
        "#0D9488", "#0f766e", "rgba(13,148,136,0.10)",
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
QMainWindow { background: qradialgradient(cx:0.3,cy:0.2,radius:1.5,stop:0 #1a1f2e,stop:1 #0f1419); }
QMenuBar { background: rgba(20,24,33,0.88); border-bottom: 1px solid rgba(255,255,255,0.06); padding: 4px 8px; }
QMenuBar::item { padding: 6px 14px; border-radius: 8px; color: #94a3b8; }
QMenuBar::item:selected { background: rgba(83,196,158,0.12); color: #53C49E; }
QMenu { background: rgba(20,24,33,0.96); border: 1px solid rgba(255,255,255,0.08); border-radius: 12px; padding: 6px; }
QMenu::item { padding: 8px 32px 8px 16px; border-radius: 8px; color: #94a3b8; }
QMenu::item:selected { background: rgba(83,196,158,0.12); color: #53C49E; }
QToolBar { background: rgba(20,24,33,0.75); border-bottom: 1px solid rgba(255,255,255,0.06); padding: 4px 8px; }
QToolBar QToolButton { padding: 6px 14px; border-radius: 8px; color: #cbd5e1; font-weight: 600; border: 1px solid rgba(255,255,255,0.06); }
QToolBar QToolButton:hover { background: rgba(255,255,255,0.08); color: #53C49E; }
QPushButton { padding: 8px 20px; border-radius: 8px; font-weight: 600; border: 1px solid transparent; min-height: 20px; }
QPushButton#primaryBtn, QPushButton[primaryBtn="true"] { background: #53C49E; color: #0f172a; font-weight: 700; border: none; }
QPushButton#primaryBtn:hover, QPushButton[primaryBtn="true"]:hover { background: #6BD5AE; }
QPushButton[flat="true"] { background: rgba(255,255,255,0.05); border: 1px solid rgba(255,255,255,0.06); color: #94a3b8; }
QPushButton[flat="true"]:hover { background: rgba(255,255,255,0.08); }
QLineEdit, QComboBox, QSpinBox { padding: 8px 14px; border-radius: 8px; border: 1px solid rgba(255,255,255,0.08); background: rgba(20,24,33,0.7); color: #e2e8f0; }
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
QListWidget { background: rgba(20,24,33,0.45); border: 1px solid rgba(255,255,255,0.05); border-radius: 12px; color: #cbd5e1; }
QListWidget::item { padding: 10px 14px; border-radius: 8px; color: #cbd5e1; }
QListWidget::item:selected { background: rgba(83,196,158,0.12); }
QScrollBar:vertical { background: rgba(255,255,255,0.03); width: 6px; border-radius: 3px; }
QScrollBar::handle:vertical { background: rgba(255,255,255,0.12); border-radius: 3px; min-height: 40px; }
QScrollBar:horizontal { background: rgba(255,255,255,0.03); height: 6px; border-radius: 3px; }
QScrollBar::handle:horizontal { background: rgba(255,255,255,0.12); border-radius: 3px; }
QProgressBar { border-radius: 6px; height: 6px; border: none; background: rgba(255,255,255,0.06); }
QProgressBar::chunk { background: #53C49E; border-radius: 6px; }
QDialog { background: qradialgradient(cx:0.4,cy:0.3,radius:1.6,stop:0 #1e2433,stop:1 #141820); border-radius: 16px; }
QToolTip { background: rgba(30,36,48,0.96); color: #e2e8f0; border: 1px solid rgba(255,255,255,0.1); border-radius: 8px; padding: 6px 12px; }
QStatusBar { background: rgba(20,24,33,0.7); border-top: 1px solid rgba(255,255,255,0.06); color: #64748b; }
        )").arg("Noto Sans SC, WenQuanYi Micro Hei, Sans Serif");
    }

    // Light themes - build from color palette
    return QString(R"(
* { font-family: "Noto Sans SC","WenQuanYi Micro Hei","Sans Serif"; font-size: 13px; color: %1; }
QMainWindow { background: qradialgradient(cx:0.3,cy:0.2,radius:1.5,stop:0 %2,stop:1 %3); }
QMenuBar { background: rgba(255,255,255,0.72); backdrop-filter: blur(20px); border-bottom: 1px solid rgba(255,255,255,0.6); padding: 4px 8px; font-weight: 500; }
QMenuBar::item { padding: 6px 14px; margin: 2px; border-radius: 8px; color: %4; }
QMenuBar::item:selected { background: %5; color: %6; }
QMenu { background: rgba(255,255,255,0.94); backdrop-filter: blur(24px); border: 1px solid rgba(255,255,255,0.7); border-radius: 12px; padding: 6px; }
QMenu::item { padding: 8px 32px 8px 16px; border-radius: 8px; color: %4; }
QMenu::item:selected { background: %5; color: %6; }
QToolBar { background: %7; backdrop-filter: blur(16px); border-bottom: 1px solid rgba(255,255,255,0.5); padding: 4px 8px; }
QToolBar QToolButton { padding: 6px 14px; border-radius: 8px; color: %4; font-weight: 600; border: 1px solid rgba(0,0,0,0.06); }
QToolBar QToolButton:hover { background: rgba(255,255,255,0.7); color: %6; }
QPushButton { padding: 8px 20px; border-radius: 8px; font-weight: 600; border: 1px solid transparent; min-height: 20px; }
QPushButton#primaryBtn, QPushButton[primaryBtn="true"] { background: %6; color: white; font-weight: 700; border: none; }
QPushButton#primaryBtn:hover, QPushButton[primaryBtn="true"]:hover { background: %8; }
QPushButton[flat="true"] { background: rgba(255,255,255,0.5); border: 1px solid rgba(255,255,255,0.4); color: %4; }
QPushButton[flat="true"]:hover { background: rgba(255,255,255,0.8); }
QLineEdit, QComboBox, QSpinBox { padding: 8px 14px; border-radius: 8px; border: 1px solid %9; background: %10; color: %1; }
QLineEdit:focus, QComboBox:focus, QSpinBox:focus { border-color: %6; }
QComboBox::drop-down { border: none; width: 24px; subcontrol-origin: padding; subcontrol-position: right center; }
QComboBox::down-arrow { width: 10px; height: 10px; }
QComboBox QAbstractItemView { background: rgba(255,255,255,0.96); border: 1px solid rgba(0,0,0,0.08); border-radius: 8px; padding: 4px; }
QComboBox QAbstractItemView::item { padding: 8px 14px; border-radius: 6px; color: %1; }
QComboBox QAbstractItemView::item:selected { background: %5; color: %6; }
QGroupBox { background: %11; backdrop-filter: blur(20px); border: 1px solid %12; border-radius: 16px; padding: 24px 16px 16px; font-weight: 700; color: %1; }
QGroupBox::title { padding: 4px 14px; margin-left: 16px; background: rgba(255,255,255,0.7); border-radius: 8px; }
QTabWidget::pane { background: rgba(255,255,255,0.45); border: 1px solid rgba(255,255,255,0.4); border-radius: 12px; }
QTabBar::tab { padding: 8px 20px; border-radius: 8px; color: %13; font-weight: 500; }
QTabBar::tab:selected { background: rgba(255,255,255,0.7); color: %6; font-weight: 700; }
QListWidget { background: rgba(255,255,255,0.5); border: 1px solid rgba(255,255,255,0.4); border-radius: 12px; color: %4; }
QListWidget::item { padding: 10px 14px; border-radius: 8px; }
QListWidget::item:selected { background: %5; color: %6; }
QScrollBar:vertical { background: rgba(0,0,0,0.03); width: 6px; border-radius: 3px; }
QScrollBar::handle:vertical { background: %14; border-radius: 3px; min-height: 40px; }
QScrollBar:horizontal { background: rgba(0,0,0,0.03); height: 6px; border-radius: 3px; }
QScrollBar::handle:horizontal { background: %14; border-radius: 3px; }
QProgressBar { border-radius: 6px; height: 6px; border: none; background: rgba(0,0,0,0.06); }
QProgressBar::chunk { background: %6; border-radius: 6px; }
QDialog { background: qradialgradient(cx:0.4,cy:0.3,radius:1.6,stop:0 %15,stop:1 %16); border-radius: 16px; }
QToolTip { background: rgba(255,255,255,0.95); color: %1; border: 1px solid rgba(0,0,0,0.08); border-radius: 8px; padding: 6px 12px; }
QStatusBar { background: %17; border-top: 1px solid rgba(255,255,255,0.5); color: %13; }
    )")
        .arg(c.textPrimary, c.bgGradientStart, c.bgGradientEnd,
             c.textSecondary, c.accentLight, c.accent, c.toolbarBg,
             c.accentHover, c.inputBorder, c.inputBg,
             c.cardBg, c.cardBorder, c.textMuted,
             c.scrollbarHandle, c.dialogBgStart, c.dialogBgEnd, c.statusBg);
}

