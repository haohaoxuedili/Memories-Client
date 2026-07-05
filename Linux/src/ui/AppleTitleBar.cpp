#include "ui/AppleTitleBar.h"
#include <QApplication>
#include <QMouseEvent>
#include <QWindow>
#include <QStyle>

AppleTitleBar::AppleTitleBar(QWidget* parentWindow, const QString& title, bool canMaximize)
    : QWidget(parentWindow)
    , m_parentWindow(parentWindow)
    , m_canMaximize(canMaximize)
{
    setFixedHeight(40);
    setCursor(Qt::ArrowCursor);
    setStyleSheet(
        "QWidget { background: rgba(255,255,255,0.18); "
        "border-bottom: 1px solid rgba(255,255,255,0.28); }");

    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(10, 0, 10, 0);
    layout->setSpacing(0);

    // ---- Left: traffic light buttons ----
    auto makeTrafficBtn = [](const QString& bg, const QString& hoverSymbol, int size) {
        auto* btn = new QPushButton();
        btn->setFixedSize(size, size);
        btn->setMinimumSize(size, size);
        btn->setMaximumSize(size, size);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setStyleSheet(QString(
            "QPushButton { background: %1; border: none; border-radius: %2px; "
            "min-width: %3px; max-width: %3px; min-height: %3px; max-height: %3px; "
            "padding: 0px; font-size: %4px; color: transparent; font-weight: 700; }"
            "QPushButton:hover { color: rgba(0,0,0,0.45); }")
            .arg(bg).arg(size/2).arg(size).arg(size*2/5));
        btn->setText(hoverSymbol);
        return btn;
    };

    m_closeBtn = makeTrafficBtn("#FF5F57", "✕", 14);
    m_minBtn   = makeTrafficBtn("#FFBD2E", "−", 14);
    m_maxBtn   = makeTrafficBtn("#28CA41", canMaximize ? "↗" : "+", 14);

    auto* trafficLayout = new QHBoxLayout();
    trafficLayout->setSpacing(8);
    trafficLayout->addWidget(m_closeBtn);
    trafficLayout->addWidget(m_minBtn);
    trafficLayout->addWidget(m_maxBtn);
    layout->addLayout(trafficLayout);

    // ---- Center: icon + title ----
    layout->addStretch();

    m_iconLabel = new QLabel();
    m_iconLabel->setFixedSize(18, 18);
    m_iconLabel->setScaledContents(true);
    m_iconLabel->setStyleSheet("background: transparent; border: none;");
    layout->addWidget(m_iconLabel);
    layout->addSpacing(6);

    m_titleLabel = new QLabel(title);
    m_titleLabel->setStyleSheet(
        "font-size: 13px; font-weight: 700; color: rgba(31,41,55,0.82); "
        "background: transparent; border: none;");
    layout->addWidget(m_titleLabel);

    layout->addStretch();

    // ---- Connections ----
    connect(m_closeBtn, &QPushButton::clicked, this, [this]() {
        m_parentWindow->close();
    });
    connect(m_minBtn, &QPushButton::clicked, this, [this]() {
        if (auto* w = m_parentWindow->window()) {
            w->showMinimized();
        }
    });
    connect(m_maxBtn, &QPushButton::clicked, this, [this]() {
        if (!m_canMaximize) return;
        if (auto* w = m_parentWindow->window()) {
            if (w->isMaximized()) {
                w->showNormal();
                m_maxBtn->setText("↗");
            } else {
                w->showMaximized();
                m_maxBtn->setText("↘");
            }
        }
    });
}

void AppleTitleBar::setTitle(const QString& title) {
    m_titleLabel->setText(title);
}

void AppleTitleBar::setIcon(const QIcon& icon) {
    m_iconLabel->setPixmap(icon.pixmap(18, 18));
}
