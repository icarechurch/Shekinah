#include "cue_card.hpp"

#include "shekinah/ui/support/asset_drag.hpp"

#include <algorithm>
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDragMoveEvent>
#include <QDropEvent>

namespace shekinah::ui {

CueGridWidget::CueCard::DropZone CueGridWidget::CueCard::drop_zone_for(QPoint pos) const {
    const int edge = width() >= 32 ? std::clamp(width() / 10, 8, 16) : 0;
    if (edge > 0 && pos.x() < edge) {
        return DropZone::insert_before;
    }
    if (edge > 0 && pos.x() >= width() - edge) {
        return DropZone::insert_after;
    }
    return DropZone::on_card;
}

void CueGridWidget::CueCard::update_drag_hover(const QMimeData* mime, QPoint pos) {
    if (!asset_id_from_mime_data(mime).has_value()) {
        setProperty("cue-drop-target", false);
        update();
        owner_.clear_insert_target();
        return;
    }

    const auto zone = drop_zone_for(pos);
    if (zone == DropZone::on_card) {
        owner_.clear_insert_target();
        setProperty("cue-drop-target", true);
        update();
    } else {
        setProperty("cue-drop-target", false);
        update();
        const std::size_t insert_index =
            zone == DropZone::insert_before ? index_ : index_ + 1;
        owner_.set_insert_target_for_card(
            index_, insert_index, zone == DropZone::insert_before);
    }
}

void CueGridWidget::CueCard::dragEnterEvent(QDragEnterEvent* event) {
    if (asset_id_from_mime_data(event->mimeData()).has_value()) {
        update_drag_hover(event->mimeData(), event->position().toPoint());
        event->acceptProposedAction();
    } else {
        setProperty("cue-drop-target", false);
        update();
        owner_.clear_insert_target();
        event->ignore();
    }
}

void CueGridWidget::CueCard::dragMoveEvent(QDragMoveEvent* event) {
    if (asset_id_from_mime_data(event->mimeData()).has_value()) {
        update_drag_hover(event->mimeData(), event->position().toPoint());
        event->acceptProposedAction();
    } else {
        setProperty("cue-drop-target", false);
        update();
        owner_.clear_insert_target();
        event->ignore();
    }
}

void CueGridWidget::CueCard::dragLeaveEvent(QDragLeaveEvent* event) {
    setProperty("cue-drop-target", false);
    update();
    owner_.clear_insert_target();
    event->accept();
}

void CueGridWidget::CueCard::dropEvent(QDropEvent* event) {
    setProperty("cue-drop-target", false);
    update();
    owner_.clear_insert_target();
    const auto asset = asset_id_from_mime_data(event->mimeData());
    if (!asset.has_value()) {
        event->ignore();
        return;
    }

    const auto zone = drop_zone_for(event->position().toPoint());
    if (zone == DropZone::on_card) {
        owner_.handle_asset_drop(cue_.id, *asset);
    } else {
        const std::size_t insert_index =
            zone == DropZone::insert_before ? index_ : index_ + 1;
        owner_.handle_asset_insert(insert_index, *asset);
    }
    event->acceptProposedAction();
}

}  // namespace shekinah::ui
