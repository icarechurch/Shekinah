#include "shekinah/ui/cue_grid_widget.hpp"

#include "cue_card.hpp"

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

}  // namespace shekinah::ui
