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

    if (dark) {
        return QString(R"(
* { font-family: "Noto Sans SC", "WenQuanYi Micro Hei", "Sans Serif"; font-size: 13px; color: #e2e8f0; }
QMainWindow { background: #0f172a; }
QMenuBar { background: #1e293b; border-bottom: 2px solid #334155; padding: 6px 12px; font-weight: 600; }
QMenuBar::item { padding: 8px 16px; margin: 2px 4px; border-radius: 8px; color: #94a3b8; }
QMenuBar::item:selected { background: #1D6E5A; color: #ffffff; }
QMenu { background: #1e293b; border: 2px solid #334155; border-radius: 12px; padding: 8px; }
QMenu::item { padding: 10px 34px 10px 18px; margin: 2px 6px; border-radius: 8px; color: #94a3b8; }
QMenu::item:selected { background: #1D6E5A; color: #ffffff; }
QToolBar { background: #1e293b; border-bottom: 2px solid #334155; padding: 8px 12px; spacing: 10px; }
QToolBar QToolButton { padding: 10px 18px; border-radius: 10px; color: #cbd5e1; font-weight: 700; border: 2px solid #334155; background: #0f172a; }
QToolBar QToolButton:hover { background: #1D6E5A; border-color: #1D6E5A; color: #ffffff; }
QPushButton { padding: 10px 24px; border-radius: 10px; font-weight: 700; border: 2px solid transparent; min-height: 24px; }
QPushButton:disabled { color: rgba(148,163,184,0.4); background: #1e293b; border-color: #334155; }
QPushButton#primaryBtn, QPushButton[primaryBtn="true"] { background: #1D6E5A; color: #ffffff; font-weight: 800; border: 2px solid #1D6E5A; }
QPushButton#primaryBtn:hover, QPushButton[primaryBtn="true"]:hover { background: #165A48; border-color: #165A48; }
QPushButton[flat="true"] { background: #1e293b; border: 2px solid #334155; color: #94a3b8; }
QPushButton[flat="true"]:hover { background: #334155; border-color: #1D6E5A; color: #ffffff; }
QLineEdit, QComboBox, QSpinBox { padding: 10px 16px; border-radius: 10px; border: 2px solid #334155; background: #0f172a; color: #e2e8f0; }
QLineEdit:focus, QComboBox:focus, QSpinBox:focus { border-color: #1D6E5A; }
QComboBox::drop-down { border: none; width: 24px; subcontrol-origin: padding; subcontrol-position: right center; }
QComboBox::down-arrow { width: 10px; height: 10px; }
QComboBox QAbstractItemView { background: #1e293b; border: 2px solid #334155; border-radius: 10px; color: #e2e8f0; padding: 8px; }
QComboBox QAbstractItemView::item { padding: 10px 16px; border-radius: 8px; }
QComboBox QAbstractItemView::item:selected { background: #1D6E5A; color: #ffffff; }
QGroupBox { background: #1e293b; border: 2px solid #334155; border-radius: 16px; margin-top: 24px; padding: 28px 20px 20px; font-weight: 700; color: #e2e8f0; }
QGroupBox::title { padding: 6px 16px; margin-left: 20px; background: #1e293b; border-radius: 10px; border: 2px solid #334155; }
QTabWidget::pane { background: #1e293b; border: 2px solid #334155; border-radius: 12px; padding: 16px; }
QTabBar::tab { padding: 10px 24px; margin: 4px 6px; border-radius: 10px; color: #64748b; font-weight: 500; }
QTabBar::tab:selected { background: #1e293b; color: #1D6E5A; font-weight: 700; border: 2px solid #334155; border-bottom: none; }
QTabBar::tab:hover:!selected { background: #334155; color: #cbd5e1; }
QListWidget { background: #1e293b; border: 2px solid #334155; border-radius: 12px; color: #cbd5e1; padding: 8px; outline: none; }
QListWidget::item { padding: 12px 16px; margin: 3px 6px; border-radius: 10px; }
QListWidget::item:hover { background: #334155; }
QListWidget::item:selected { background: #1D6E5A; color: #ffffff; }
QScrollBar:vertical { background: #1e293b; width: 10px; border-radius: 5px; margin: 4px 0; }
QScrollBar::handle:vertical { background: #475569; border-radius: 5px; min-height: 40px; }
QScrollBar::handle:vertical:hover { background: #64748b; }
QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; border: none; }
QScrollBar:horizontal { background: #1e293b; height: 10px; border-radius: 5px; margin: 0 4px; }
QScrollBar::handle:horizontal { background: #475569; border-radius: 5px; min-width: 40px; }
QLabel[galleryState="true"] { color: #94a3b8; font-size: 16px; font-weight: 600; padding: 24px; }
QLabel[galleryMeta="true"] { color: #64748b; font-size: 12px; font-weight: 700; padding: 0 6px; }
QToolButton[galleryThumb="true"] { background: #1e293b; border-radius: 12px; border: 2px solid #334155; padding: 6px; }
QToolButton[galleryThumb="true"]:hover { background: #334155; border-color: #1D6E5A; }
QToolButton[galleryThumb="true"][selected="true"] { background: #1e293b; border: 3px solid #1D6E5A; }
QProgressBar { min-height: 12px; max-height: 12px; border-radius: 6px; border: 2px solid #334155; background: #0f172a; }
QProgressBar::chunk { border-radius: 6px; background: #53C49E; }
QDialog { background: #1e293b; border: 2px solid #334155; border-radius: 16px; }
QToolTip { background: #0f172a; color: #e2e8f0; border: 2px solid #334155; border-radius: 10px; padding: 8px 14px; }
QStatusBar { background: #1e293b; border-top: 2px solid #334155; color: #64748b; padding: 6px 14px; }
QLabel[statusText="true"] { color: #94a3b8; font-size: 12px; }
QLabel[statusPill="checking"] { color: #FBBF24; background: rgba(251,191,36,0.15); border: 2px solid rgba(251,191,36,0.4); border-radius: 10px; padding: 4px 12px; font-weight: 700; }
QLabel[statusPill="ok"] { color: #53C49E; background: rgba(83,196,158,0.15); border: 2px solid rgba(83,196,158,0.4); border-radius: 10px; padding: 4px 12px; font-weight: 700; }
QLabel[statusPill="error"] { color: #FB7185; background: rgba(251,113,133,0.15); border: 2px solid rgba(251,113,133,0.4); border-radius: 10px; padding: 4px 12px; font-weight: 700; }
QTextEdit { background: #1e293b; border: 2px solid #334155; border-radius: 12px; padding: 14px 18px; color: #cbd5e1; }
QCheckBox { spacing: 12px; color: #cbd5e1; font-size: 13px; }
QCheckBox::indicator { width: 22px; height: 22px; border-radius: 6px; border: 2px solid #475569; background: #0f172a; }
QCheckBox::indicator:checked { background: #1D6E5A; border-color: #1D6E5A; }
        )");
    }

    return QString(R"(
* { font-family: "Noto Sans SC", "WenQuanYi Micro Hei", "Sans Serif"; font-size: 13px; color: #1e293b; }
QMainWindow { background: #f1f5f9; }
QMenuBar { background: #ffffff; border-bottom: 2px solid #cbd5e1; padding: 6px 12px; font-weight: 600; }
QMenuBar::item { padding: 8px 16px; margin: 2px 4px; border-radius: 8px; color: #334155; }
QMenuBar::item:selected { background: #ecfdf5; color: #1D6E5A; }
QMenu { background: #ffffff; border: 2px solid #cbd5e1; border-radius: 12px; padding: 8px; }
QMenu::item { padding: 10px 34px 10px 18px; margin: 2px 6px; border-radius: 8px; color: #334155; }
QMenu::item:selected { background: #ecfdf5; color: #1D6E5A; }
QToolBar { background: #ffffff; border-bottom: 2px solid #cbd5e1; padding: 8px 12px; spacing: 10px; }
QToolBar QToolButton { padding: 10px 18px; border-radius: 10px; color: #334155; font-weight: 700; border: 2px solid #cbd5e1; background: #f8fafc; }
QToolBar QToolButton:hover { background: #ecfdf5; border-color: #1D6E5A; color: #1D6E5A; }
QPushButton { padding: 10px 24px; border-radius: 10px; font-weight: 700; border: 2px solid transparent; min-height: 24px; }
QPushButton:disabled { color: rgba(100,116,139,0.4); background: #f1f5f9; border-color: #e2e8f0; }
QPushButton#primaryBtn, QPushButton[primaryBtn="true"] { background: #1D6E5A; color: #ffffff; font-weight: 800; border: 2px solid #1D6E5A; }
QPushButton#primaryBtn:hover, QPushButton[primaryBtn="true"]:hover { background: #165A48; border-color: #165A48; }
QPushButton[flat="true"] { background: #f8fafc; border: 2px solid #cbd5e1; color: #475569; }
QPushButton[flat="true"]:hover { background: #f1f5f9; border-color: #1D6E5A; color: #1D6E5A; }
QPushButton { background: #ffffff; border: 2px solid #cbd5e1; color: #334155; }
QPushButton:hover { border-color: #1D6E5A; color: #1D6E5A; }
QPushButton:pressed { background: #f1f5f9; }
QLineEdit, QComboBox, QSpinBox { padding: 10px 16px; border-radius: 10px; border: 2px solid #cbd5e1; background: #ffffff; color: #1e293b; }
QLineEdit:focus, QComboBox:focus, QSpinBox:focus { border-color: #1D6E5A; }
QComboBox::drop-down { border: none; width: 24px; subcontrol-origin: padding; subcontrol-position: right center; }
QComboBox::down-arrow { width: 10px; height: 10px; }
QComboBox QAbstractItemView { background: #ffffff; border: 2px solid #cbd5e1; border-radius: 10px; padding: 8px; }
QComboBox QAbstractItemView::item { padding: 10px 16px; border-radius: 8px; color: #1e293b; }
QComboBox QAbstractItemView::item:selected { background: #ecfdf5; color: #1D6E5A; }
QGroupBox { background: #ffffff; border: 2px solid #94a3b8; border-radius: 16px; margin-top: 24px; padding: 28px 20px 20px; font-weight: 700; color: #1e293b; }
QGroupBox::title { padding: 6px 16px; margin-left: 20px; background: #ffffff; border-radius: 10px; border: 2px solid #94a3b8; }
QTabWidget::pane { background: #ffffff; border: 2px solid #94a3b8; border-radius: 12px; padding: 16px; }
QTabBar::tab { padding: 10px 24px; margin: 4px 6px; border-radius: 10px; color: #64748b; font-weight: 500; }
QTabBar::tab:selected { background: #ffffff; color: #1D6E5A; font-weight: 700; border: 2px solid #94a3b8; border-bottom: none; }
QTabBar::tab:hover:!selected { background: #f1f5f9; color: #334155; }
QListWidget { background: #ffffff; border: 2px solid #cbd5e1; border-radius: 12px; color: #334155; padding: 8px; outline: none; }
QListWidget::item { padding: 12px 16px; margin: 3px 6px; border-radius: 10px; }
QListWidget::item:hover { background: #f1f5f9; }
QListWidget::item:selected { background: #ecfdf5; color: #1D6E5A; }
QScrollBar:vertical { background: #e2e8f0; width: 10px; border-radius: 5px; margin: 4px 0; }
QScrollBar::handle:vertical { background: #94a3b8; border-radius: 5px; min-height: 40px; }
QScrollBar::handle:vertical:hover { background: #64748b; }
QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; border: none; }
QScrollBar:horizontal { background: #e2e8f0; height: 10px; border-radius: 5px; margin: 0 4px; }
QScrollBar::handle:horizontal { background: #94a3b8; border-radius: 5px; min-width: 40px; }
QLabel[galleryState="true"] { color: #64748b; font-size: 16px; font-weight: 600; padding: 24px; }
QLabel[galleryMeta="true"] { color: #64748b; font-size: 12px; font-weight: 700; padding: 0 6px; }
QToolButton[galleryThumb="true"] { background: #ffffff; border-radius: 12px; border: 2px solid #cbd5e1; padding: 6px; }
QToolButton[galleryThumb="true"]:hover { border-color: #1D6E5A; }
QToolButton[galleryThumb="true"][selected="true"] { border: 3px solid #1D6E5A; }
QProgressBar { min-height: 12px; max-height: 12px; border-radius: 6px; border: 2px solid #cbd5e1; background: #f1f5f9; }
QProgressBar::chunk { border-radius: 6px; background: #53C49E; }
QDialog { background: #f8fafc; border: 2px solid #94a3b8; border-radius: 16px; }
QToolTip { background: #ffffff; color: #1e293b; border: 2px solid #94a3b8; border-radius: 10px; padding: 8px 14px; }
QStatusBar { background: #ffffff; border-top: 2px solid #cbd5e1; color: #64748b; padding: 6px 14px; }
QLabel[statusText="true"] { color: #64748b; font-size: 12px; }
QLabel[statusPill="checking"] { color: #B45309; background: rgba(245,158,11,0.15); border: 2px solid rgba(245,158,11,0.4); border-radius: 10px; padding: 4px 12px; font-weight: 700; }
QLabel[statusPill="ok"] { color: #1D6E5A; background: rgba(83,196,158,0.15); border: 2px solid rgba(83,196,158,0.4); border-radius: 10px; padding: 4px 12px; font-weight: 700; }
QLabel[statusPill="error"] { color: #E11D48; background: rgba(251,113,133,0.15); border: 2px solid rgba(251,113,133,0.4); border-radius: 10px; padding: 4px 12px; font-weight: 700; }
QTextEdit { background: #ffffff; border: 2px solid #cbd5e1; border-radius: 12px; padding: 14px 18px; color: #475569; }
QCheckBox { spacing: 12px; color: #334155; font-size: 13px; }
QCheckBox::indicator { width: 22px; height: 22px; border-radius: 6px; border: 2px solid #94a3b8; background: #ffffff; }
QCheckBox::indicator:checked { background: #1D6E5A; border-color: #1D6E5A; }
QLabel[heading="true"] { font-weight: 700; font-size: 20px; color: #0f172a; }
QLabel[subtitle="true"] { font-size: 12px; color: #64748b; }
QLabel#appTitle { font-weight: 800; font-size: 32px; color: #0f172a; }
    )");
}

