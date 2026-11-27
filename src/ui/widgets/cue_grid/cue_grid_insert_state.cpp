#include "shekinah/ui/cue_grid_widget.hpp"

#include "cue_grid_overlay.hpp"

namespace shekinah::ui {

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
