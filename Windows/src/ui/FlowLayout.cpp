#include "ui/FlowLayout.h"
#include <QWidget>
#include <QStyle>

FlowLayout::FlowLayout(QWidget* parent, int margin, int spacing)
    : QLayout(parent), m_hSpace(spacing), m_vSpace(spacing)
{
    setContentsMargins(margin, margin, margin, margin);
}

FlowLayout::~FlowLayout() {
    while (auto* item = takeAt(0))
        delete item;
}

void FlowLayout::addItem(QLayoutItem* item) {
    m_items.append(item);
}

QLayoutItem* FlowLayout::itemAt(int index) const {
    return (index >= 0 && index < m_items.size()) ? m_items[index] : nullptr;
}

QLayoutItem* FlowLayout::takeAt(int index) {
    return (index >= 0 && index < m_items.size()) ? m_items.takeAt(index) : nullptr;
}

int FlowLayout::horizontalSpacing() const {
    if (m_hSpace >= 0) return m_hSpace;
    return smartSpacing(QStyle::PM_LayoutHorizontalSpacing);
}

int FlowLayout::verticalSpacing() const {
    if (m_vSpace >= 0) return m_vSpace;
    return smartSpacing(QStyle::PM_LayoutVerticalSpacing);
}

int FlowLayout::heightForWidth(int width) const {
    return doLayout(QRect(0, 0, width, 0), true);
}

QSize FlowLayout::minimumSize() const {
    QSize size;
    for (auto* item : m_items)
        size = size.expandedTo(item->minimumSize());
    int m = contentsMargins().left() + contentsMargins().right();
    return size + QSize(m, m);
}

QSize FlowLayout::sizeHint() const {
    return minimumSize();
}

void FlowLayout::setGeometry(const QRect& rect) {
    QLayout::setGeometry(rect);
    doLayout(rect, false);
}

int FlowLayout::doLayout(const QRect& rect, bool testOnly) const {
    int left, top, right, bottom;
    getContentsMargins(&left, &top, &right, &bottom);
    QRect effectiveRect = rect.adjusted(left, top, -right, -bottom);
    int x = effectiveRect.x();
    int y = effectiveRect.y();
    int lineHeight = 0;
    int maxWidth = effectiveRect.width();
    int cellSize = m_cellSize;
    int hSpacing = horizontalSpacing();
    int vSpacing = verticalSpacing();

    for (auto* item : m_items) {
        auto* w = item->widget();
        int span = w ? w->property("span").toInt() : 1;
        int itemWidth = cellSize * span + (span - 1) * hSpacing;
        int itemHeight = cellSize;

        // Wrap to next line if needed
        if (x + itemWidth > effectiveRect.right() + 1 && lineHeight > 0) {
            x = effectiveRect.x();
            y += lineHeight + vSpacing;
            lineHeight = 0;
        }

        if (!testOnly) {
            item->setGeometry(QRect(QPoint(x, y), QSize(itemWidth, itemHeight)));
        }

        x += itemWidth + hSpacing;
        lineHeight = qMax(lineHeight, itemHeight);
    }

    return y + lineHeight - rect.y() + bottom;
}

int FlowLayout::smartSpacing(QStyle::PixelMetric pm) const {
    auto* p = parentWidget();
    if (!p) return 0;
    return p->style()->pixelMetric(pm, nullptr, p);
}
