#include "service_order_list.hpp"
#include "service_order_primitives.hpp"

#include "shekinah/ui/support/asset_drag.hpp"
#include "shekinah/ui/support/content_drag.hpp"

#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QEvent>
#include <QMimeData>

namespace shekinah::ui::service_order_internal {

void ServiceOrderListWidget::set_drop_hover_state(const QMimeData* data, QPoint pos) {
    clear_drop_hover_state();
    if (asset_id_from_mime_data(data).has_value()) {
        if (auto* target = itemAt(pos); target != nullptr) {
            target->setData(kDropTargetRole, true);
            hovered_drop_row_ = row(target);
        }
    } else if (content_ref_from_mime_data(data).has_value()) {
        content_drop_active_ = true;
    }
    viewport()->update();
}

void ServiceOrderListWidget::clear_drop_hover_state() {
    if (hovered_drop_row_ >= 0 && hovered_drop_row_ < count()) {
        item(hovered_drop_row_)->setData(kDropTargetRole, false);
    }
    hovered_drop_row_ = -1;
    content_drop_active_ = false;
    viewport()->update();
}

bool ServiceOrderListWidget::handle_drop(const QMimeData* data, QPoint position) {
    clear_drop_hover_state();
    if (asset_id_from_mime_data(data).has_value()) {
        return asset_drop_handler_ && asset_drop_handler_(data, position);
    }
    if (content_ref_from_mime_data(data).has_value()) {
        return content_drop_handler_ && content_drop_handler_(data);
    }
    return false;
}

bool ServiceOrderListWidget::accepts_drop(const QMimeData* data) const {
    return asset_id_from_mime_data(data).has_value() ||
           content_ref_from_mime_data(data).has_value();
}

bool ServiceOrderListWidget::eventFilter(QObject* watched, QEvent* event) {
    if (watched == viewport()) {
        if (event->type() == QEvent::Resize) {
            if (viewport_resized_) {
                viewport_resized_();
            }
        } else if (event->type() == QEvent::Drop) {
            auto* drop_event = static_cast<QDropEvent*>(event);
            if (!handle_drop(drop_event->mimeData(), drop_event->position().toPoint())) {
                drop_event->ignore();
            } else {
                drop_event->acceptProposedAction();
            }
            return true;
        } else if (event->type() == QEvent::DragEnter) {
            auto* drag_event = static_cast<QDragEnterEvent*>(event);
            if (accepts_drop(drag_event->mimeData())) {
                set_drop_hover_state(drag_event->mimeData(), drag_event->position().toPoint());
                drag_event->acceptProposedAction();
            } else {
                clear_drop_hover_state();
                drag_event->ignore();
            }
            return true;
        } else if (event->type() == QEvent::DragMove) {
            auto* drag_event = static_cast<QDragMoveEvent*>(event);
            if (accepts_drop(drag_event->mimeData())) {
                set_drop_hover_state(drag_event->mimeData(), drag_event->position().toPoint());
                drag_event->acceptProposedAction();
            } else {
                clear_drop_hover_state();
                drag_event->ignore();
            }
            return true;
        } else if (event->type() == QEvent::DragLeave) {
            clear_drop_hover_state();
            return true;
        }
    }
    return QListWidget::eventFilter(watched, event);
}

bool ServiceOrderListWidget::viewportEvent(QEvent* event) {
    if (event->type() == QEvent::DragEnter) {
        auto* drag_event = static_cast<QDragEnterEvent*>(event);
        if (accepts_drop(drag_event->mimeData())) {
            set_drop_hover_state(drag_event->mimeData(), drag_event->position().toPoint());
            drag_event->acceptProposedAction();
        } else {
            clear_drop_hover_state();
            drag_event->ignore();
        }
        return true;
    }
    if (event->type() == QEvent::DragMove) {
        auto* drag_event = static_cast<QDragMoveEvent*>(event);
        if (accepts_drop(drag_event->mimeData())) {
            set_drop_hover_state(drag_event->mimeData(), drag_event->position().toPoint());
            drag_event->acceptProposedAction();
        } else {
            clear_drop_hover_state();
            drag_event->ignore();
        }
        return true;
    }
    if (event->type() == QEvent::DragLeave) {
        clear_drop_hover_state();
        return true;
    }
    if (event->type() == QEvent::Drop) {
        auto* drop_event = static_cast<QDropEvent*>(event);
        if (!handle_drop(drop_event->mimeData(), drop_event->position().toPoint())) {
            drop_event->ignore();
        } else {
            drop_event->acceptProposedAction();
        }
        return true;
    }
    return QListWidget::viewportEvent(event);
}

void ServiceOrderListWidget::dragEnterEvent(QDragEnterEvent* event) {
    if (accepts_drop(event->mimeData())) {
        set_drop_hover_state(event->mimeData(), event->position().toPoint());
        event->acceptProposedAction();
    } else {
        clear_drop_hover_state();
        event->ignore();
    }
}

void ServiceOrderListWidget::dragMoveEvent(QDragMoveEvent* event) {
    if (accepts_drop(event->mimeData())) {
        set_drop_hover_state(event->mimeData(), event->position().toPoint());
        event->acceptProposedAction();
    } else {
        clear_drop_hover_state();
        event->ignore();
    }
}

void ServiceOrderListWidget::dragLeaveEvent(QDragLeaveEvent* event) {
    clear_drop_hover_state();
    event->accept();
}

void ServiceOrderListWidget::dropEvent(QDropEvent* event) {
    if (!handle_drop(event->mimeData(), event->position().toPoint())) {
        event->ignore();
        return;
    }
    event->acceptProposedAction();
}

}  // namespace shekinah::ui::service_order_internal
