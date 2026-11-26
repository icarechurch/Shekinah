#include "shekinah/ui/cue_grid_widget.hpp"

#include "cue_grid_overlay.hpp"
#include "shekinah/ui/support/asset_drag.hpp"

#include <QDragLeaveEvent>
#include <QDropEvent>
#include <QEvent>
#include <QResizeEvent>
#include <QScrollArea>

namespace shekinah::ui {

bool CueGridWidget::eventFilter(QObject* watched, QEvent* event) {
    if (grid_scroll_ != nullptr && watched == grid_scroll_->viewport() &&
        event->type() == QEvent::Resize) {
        relayout_cards(false);
    }
    if (grid_container_ != nullptr && watched == grid_container_ &&
        event->type() == QEvent::Resize && insert_overlay_ != nullptr) {
        insert_overlay_->setGeometry(grid_container_->rect());
    }

    const bool is_container = (grid_container_ != nullptr && watched == grid_container_);
    const bool is_viewport =
        (grid_scroll_ != nullptr && watched == grid_scroll_->viewport());
    if (is_container || is_viewport) {
        const auto to_container_pos = [&](QPoint pos) {
            return is_container ? pos : grid_container_->mapFrom(grid_scroll_->viewport(), pos);
        };
        if (event->type() == QEvent::DragEnter || event->type() == QEvent::DragMove) {
            auto* drag_event = static_cast<QDropEvent*>(event);
            if (asset_id_from_mime_data(drag_event->mimeData()).has_value() && !cards_.empty()) {
                if (const auto target = compute_insert_target(
                        to_container_pos(drag_event->position().toPoint()));
                    target.has_value()) {
                    set_insert_target(*target);
                    drag_event->acceptProposedAction();
                    return true;
                }
            }
            clear_insert_target();
            drag_event->ignore();
            return true;
        }
        if (event->type() == QEvent::DragLeave) {
            clear_insert_target();
            return true;
        }
        if (event->type() == QEvent::Drop) {
            auto* drop_event = static_cast<QDropEvent*>(event);
            const auto asset = asset_id_from_mime_data(drop_event->mimeData());
            const auto target = cards_.empty()
                                    ? std::nullopt
                                    : compute_insert_target(
                                          to_container_pos(drop_event->position().toPoint()));
            clear_insert_target();
            if (!asset.has_value() || !target.has_value()) {
                drop_event->ignore();
                return true;
            }
            handle_asset_insert(target->index, *asset);
            drop_event->acceptProposedAction();
            return true;
        }
    }
    return QWidget::eventFilter(watched, event);
}

void CueGridWidget::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    relayout_cards(false);
}

void CueGridWidget::handle_left_click(const domain::CueId& cue) {
    if (take_live_handler_) {
        take_live_handler_(cue);
    }
}

void CueGridWidget::handle_right_click(const domain::CueId& cue, QPoint global_position) {
    if (context_handler_) {
        context_handler_(cue, global_position);
    }
}

void CueGridWidget::handle_asset_drop(const domain::CueId& cue, const domain::AssetId& asset) {
    if (asset_drop_handler_) {
        asset_drop_handler_(cue, asset);
    }
}

void CueGridWidget::handle_asset_insert(std::size_t index, const domain::AssetId& asset) {
    if (asset_insert_handler_) {
        asset_insert_handler_(index, asset);
    }
}

}  // namespace shekinah::ui
