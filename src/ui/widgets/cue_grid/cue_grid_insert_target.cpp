#include "shekinah/ui/cue_grid_widget.hpp"

#include "cue_card.hpp"
#include "cue_grid_overlay.hpp"

#include <algorithm>

namespace shekinah::ui {
namespace {

constexpr int kGridSpacing = 12;

}  // namespace

QRect CueGridWidget::indicator_rect_for_card_edge(std::size_t card_index, bool left_side) const {
    if (card_index >= cards_.size() || grid_container_ == nullptr) {
        return {};
    }
    const auto* card = cards_[card_index];
    int x = 0;
    if (left_side) {
        if (card_index > 0 && cards_[card_index - 1]->y() == card->y()) {
            const int prev_right = cards_[card_index - 1]->geometry().right() + 1;
            x = (prev_right + card->x()) / 2;
        } else {
            x = std::max(3, card->x() / 2);
        }
    } else {
        if (card_index + 1 < cards_.size() && cards_[card_index + 1]->y() == card->y()) {
            const int card_right = card->geometry().right() + 1;
            x = (card_right + cards_[card_index + 1]->x()) / 2;
        } else {
            const int card_right = card->geometry().right() + 1;
            x = std::min(std::max(3, grid_container_->width() - 4),
                         card_right + std::max(3, kGridSpacing / 2));
        }
    }
    return QRect{x - 2, card->y(), 4, card->height()};
}

std::optional<CueGridWidget::InsertTarget> CueGridWidget::compute_insert_target(
    QPoint container_pos) const {
    if (cards_.empty() || grid_container_ == nullptr) {
        return std::nullopt;
    }
    const std::size_t count = cards_.size();
    const std::size_t columns = static_cast<std::size_t>(std::max(1, current_columns_));
    const std::size_t row_count = (count + columns - 1) / columns;

    const auto row_bounds = [&](std::size_t row) -> std::pair<int, int> {
        const std::size_t start = row * columns;
        const std::size_t end = std::min((row + 1) * columns, count);
        int top = cards_[start]->y();
        int bottom = cards_[start]->geometry().bottom();
        for (std::size_t i = start + 1; i < end; ++i) {
            top = std::min(top, cards_[i]->y());
            bottom = std::max(bottom, cards_[i]->geometry().bottom());
        }
        return {top, bottom};
    };

    if (columns == 1) {
        if (container_pos.y() < cards_[0]->y()) {
            if (container_pos.x() < cards_[0]->x()) {
                return InsertTarget{0, indicator_rect_for_card_edge(0, true)};
            }
            const int y = std::max(2, cards_[0]->y() / 2);
            return InsertTarget{0, QRect{cards_[0]->x(), y - 2, cards_[0]->width(), 4}};
        }
        for (std::size_t r = 0; r + 1 < row_count; ++r) {
            const int bottom = cards_[r]->geometry().bottom();
            const int next_top = cards_[r + 1]->y();
            if (container_pos.y() > bottom && container_pos.y() < next_top) {
                const int mid_y = (bottom + 1 + next_top) / 2;
                return InsertTarget{
                    r + 1, QRect{cards_[r]->x(), mid_y - 2, cards_[r]->width(), 4}};
            }
        }
        if (container_pos.y() > cards_.back()->geometry().bottom()) {
            const int y = std::min(std::max(2, grid_container_->height() - 4),
                                   cards_.back()->geometry().bottom() + 1 + kGridSpacing / 2);
            return InsertTarget{
                count, QRect{cards_.back()->x(), y - 2, cards_.back()->width(), 4}};
        }
    }

    const auto last_bounds = row_bounds(row_count - 1);
    if (container_pos.y() > last_bounds.second) {
        return InsertTarget{count, indicator_rect_for_card_edge(count - 1, false)};
    }

    std::size_t target_row = 0;
    if (container_pos.y() > row_bounds(0).second) {
        for (std::size_t r = 0; r < row_count; ++r) {
            const auto bounds = row_bounds(r);
            if (container_pos.y() >= bounds.first && container_pos.y() <= bounds.second) {
                target_row = r;
                break;
            }
            if (r + 1 < row_count) {
                const auto next_bounds = row_bounds(r + 1);
                if (container_pos.y() > bounds.second && container_pos.y() < next_bounds.first) {
                    const int mid_y = (bounds.second + 1 + next_bounds.first) / 2;
                    target_row = container_pos.y() <= mid_y ? r : (r + 1);
                    break;
                }
            }
        }
    }

    const std::size_t row_start = target_row * columns;
    const std::size_t row_end = std::min((target_row + 1) * columns, count);
    if (container_pos.x() < cards_[row_start]->geometry().center().x()) {
        return InsertTarget{row_start, indicator_rect_for_card_edge(row_start, true)};
    }
    for (std::size_t idx = row_start; idx + 1 < row_end; ++idx) {
        if (container_pos.x() < cards_[idx + 1]->geometry().center().x()) {
            return InsertTarget{idx + 1, indicator_rect_for_card_edge(idx, false)};
        }
    }
    return InsertTarget{row_end, indicator_rect_for_card_edge(row_end - 1, false)};
}

void CueGridWidget::set_insert_target(InsertTarget target) {
    insert_target_ = target;
    setProperty("cue-insert-index", static_cast<int>(target.index));
    if (insert_overlay_ != nullptr) {
        insert_overlay_->set_indicator_rect(target.indicator_rect);
    }
}

void CueGridWidget::set_insert_target_for_card(
    std::size_t card_index, std::size_t insert_index, bool left_side) {
    set_insert_target(InsertTarget{
        insert_index,
        indicator_rect_for_card_edge(card_index, left_side),
    });
}

void CueGridWidget::clear_insert_target() {
    insert_target_.reset();
    setProperty("cue-insert-index", -1);
    if (insert_overlay_ != nullptr) {
        insert_overlay_->set_indicator_rect({});
    }
}

}  // namespace shekinah::ui
