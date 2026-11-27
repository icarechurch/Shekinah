#include "service_order_list.hpp"
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QEvent>
#include <QMimeData>

namespace shekinah::ui::service_order_internal {

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
