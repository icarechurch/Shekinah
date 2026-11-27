#include "shekinah/ui/support/flow_layout.hpp"

#include <QStyle>
#include <QWidget>

#include <algorithm>

namespace shekinah::ui {

FlowLayout::FlowLayout(QWidget* parent, int margin, int h_spacing, int v_spacing)
    : QLayout(parent), h_space_(h_spacing), v_space_(v_spacing) {
    if (margin >= 0) {
        setContentsMargins(margin, margin, margin, margin);
    }
}

FlowLayout::~FlowLayout() {
    while (auto* item = takeAt(0)) {
        delete item;
    }
}

void FlowLayout::addItem(QLayoutItem* item) { items_.push_back(item); }

int FlowLayout::horizontalSpacing() const {
    return h_space_ >= 0 ? h_space_ : smart_spacing(QStyle::PM_LayoutHorizontalSpacing);
}

int FlowLayout::verticalSpacing() const {
    return v_space_ >= 0 ? v_space_ : smart_spacing(QStyle::PM_LayoutVerticalSpacing);
}

Qt::Orientations FlowLayout::expandingDirections() const { return {}; }

bool FlowLayout::hasHeightForWidth() const { return true; }

int FlowLayout::heightForWidth(int width) const {
    return do_layout(QRect{0, 0, width, 0}, true);
}

int FlowLayout::count() const { return static_cast<int>(items_.size()); }

QLayoutItem* FlowLayout::itemAt(int index) const {
    if (index < 0 || index >= count()) {
        return nullptr;
    }
    return items_[static_cast<std::size_t>(index)];
}

QSize FlowLayout::minimumSize() const {
    QSize size;
    for (const auto* item : items_) {
        if (is_item_hidden(item)) {
            continue;
        }
        size = size.expandedTo(item->minimumSize());
    }
    const auto margins = contentsMargins();
    size += QSize{margins.left() + margins.right(), margins.top() + margins.bottom()};
    return size;
}

void FlowLayout::setGeometry(const QRect& rect) {
    QLayout::setGeometry(rect);
    do_layout(rect, false);
}

QSize FlowLayout::sizeHint() const {
    int total_width = 0;
    int max_height = 0;
    int visible_count = 0;
    const int space_x = horizontalSpacing();
    for (const auto* item : items_) {
        if (is_item_hidden(item)) {
            continue;
        }
        const QSize hint = item->sizeHint();
        if (visible_count > 0) {
            total_width += space_x;
        }
        total_width += hint.width();
        max_height = std::max(max_height, hint.height());
        ++visible_count;
    }
    const auto margins = contentsMargins();
    return {total_width + margins.left() + margins.right(),
            max_height + margins.top() + margins.bottom()};
}

QLayoutItem* FlowLayout::takeAt(int index) {
    if (index < 0 || index >= count()) {
        return nullptr;
    }
    auto* item = items_[static_cast<std::size_t>(index)];
    items_.erase(items_.begin() + index);
    return item;
}

bool FlowLayout::is_item_hidden(const QLayoutItem* item) {
    if (item == nullptr) {
        return true;
    }
    if (const auto* widget = item->widget(); widget != nullptr) {
        return widget->isHidden();
    }
    return item->isEmpty();
}

int FlowLayout::smart_spacing(int metric) const {
    QObject* obj = parent();
    if (obj == nullptr) {
        return -1;
    }
    if (obj->isWidgetType()) {
        const auto* pw = static_cast<const QWidget*>(obj);
        return pw->style()->pixelMetric(static_cast<QStyle::PixelMetric>(metric), nullptr, pw);
    }
    return static_cast<const QLayout*>(obj)->spacing();
}

int FlowLayout::do_layout(const QRect& rect, bool test_only) const {
    const auto margins = contentsMargins();
    const QRect effective =
        rect.adjusted(margins.left(), margins.top(), -margins.right(), -margins.bottom());
    const int space_x = std::max(0, horizontalSpacing());
    const int space_y = std::max(0, verticalSpacing());

    struct Placement {
        QLayoutItem* item;
        int x;
        int y;
        QSize size;
        int row_index;
    };

    std::vector<Placement> placements;
    std::vector<int> row_heights;
    int x = effective.x();
    int y = effective.y();
    int line_height = 0;
    int row_index = 0;
    bool first_in_row = true;

    for (auto* item : items_) {
        if (is_item_hidden(item)) {
            continue;
        }
        const QSize hint = item->sizeHint();
        const int needed_x = first_in_row ? hint.width() : (space_x + hint.width());
        if (!first_in_row && x + needed_x > effective.right() + 1) {
            row_heights.push_back(line_height);
            x = effective.x();
            y += line_height + space_y;
            line_height = 0;
            ++row_index;
            first_in_row = true;
        }
        if (!first_in_row) {
            x += space_x;
        }
        const int draw_w = std::min(hint.width(), std::max(0, effective.width()));
        placements.push_back(Placement{item, x, y, QSize{draw_w, hint.height()}, row_index});
        x += draw_w;
        line_height = std::max(line_height, hint.height());
        first_in_row = false;
    }
    row_heights.push_back(line_height);

    if (!test_only) {
        for (const auto& p : placements) {
            const int rh = row_heights[static_cast<std::size_t>(p.row_index)];
            const int centered_y = p.y + std::max(0, (rh - p.size.height()) / 2);
            p.item->setGeometry(QRect{QPoint{p.x, centered_y}, p.size});
        }
    }

    return y + line_height - rect.y() + margins.bottom();
}

}  // namespace shekinah::ui
