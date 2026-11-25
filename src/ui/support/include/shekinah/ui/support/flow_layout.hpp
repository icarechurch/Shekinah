#pragma once

#include <QLayout>
#include <QRect>
#include <QStyle>
#include <QWidget>

#include <algorithm>
#include <vector>

namespace shekinah::ui {

/// Responsive wrapping horizontal layout for toolbars that adapt to narrow windows.
class FlowLayout final : public QLayout {
public:
    explicit FlowLayout(QWidget* parent = nullptr,
                        int margin = -1,
                        int h_spacing = -1,
                        int v_spacing = -1)
        : QLayout(parent), h_space_(h_spacing), v_space_(v_spacing) {
        if (margin >= 0) {
            setContentsMargins(margin, margin, margin, margin);
        }
    }

    ~FlowLayout() override {
        while (auto* item = takeAt(0)) {
            delete item;
        }
    }

    void addItem(QLayoutItem* item) override { items_.push_back(item); }

    [[nodiscard]] int horizontalSpacing() const {
        return h_space_ >= 0 ? h_space_ : smart_spacing(QStyle::PM_LayoutHorizontalSpacing);
    }

    [[nodiscard]] int verticalSpacing() const {
        return v_space_ >= 0 ? v_space_ : smart_spacing(QStyle::PM_LayoutVerticalSpacing);
    }

    [[nodiscard]] Qt::Orientations expandingDirections() const override { return {}; }
    [[nodiscard]] bool hasHeightForWidth() const override { return true; }

    [[nodiscard]] int heightForWidth(int width) const override {
        return do_layout(QRect{0, 0, width, 0}, true);
    }

    [[nodiscard]] int count() const override { return static_cast<int>(items_.size()); }

    [[nodiscard]] QLayoutItem* itemAt(int index) const override {
        if (index < 0 || index >= count()) {
            return nullptr;
        }
        return items_[static_cast<std::size_t>(index)];
    }

    [[nodiscard]] QSize minimumSize() const override {
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

    void setGeometry(const QRect& rect) override {
        QLayout::setGeometry(rect);
        do_layout(rect, false);
    }

    [[nodiscard]] QSize sizeHint() const override {
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

    [[nodiscard]] QLayoutItem* takeAt(int index) override {
        if (index < 0 || index >= count()) {
            return nullptr;
        }
        auto* item = items_[static_cast<std::size_t>(index)];
        items_.erase(items_.begin() + index);
        return item;
    }

private:
    static bool is_item_hidden(const QLayoutItem* item) {
        if (item == nullptr) {
            return true;
        }
        if (const auto* widget = item->widget(); widget != nullptr) {
            return widget->isHidden();
        }
        return item->isEmpty();
    }

    [[nodiscard]] int smart_spacing(QStyle::PixelMetric metric) const {
        QObject* obj = parent();
        if (obj == nullptr) {
            return -1;
        }
        if (obj->isWidgetType()) {
            const auto* pw = static_cast<const QWidget*>(obj);
            return pw->style()->pixelMetric(metric, nullptr, pw);
        }
        return static_cast<const QLayout*>(obj)->spacing();
    }

    int do_layout(const QRect& rect, bool test_only) const {
        const auto margins = contentsMargins();
        const QRect effective = rect.adjusted(margins.left(), margins.top(), -margins.right(), -margins.bottom());
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

    std::vector<QLayoutItem*> items_;
    int h_space_{-1};
    int v_space_{-1};
};

}  // namespace shekinah::ui
