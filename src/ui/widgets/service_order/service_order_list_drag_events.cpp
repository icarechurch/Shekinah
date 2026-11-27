#include "service_order_list.hpp"

#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QMimeData>

namespace shekinah::ui::service_order_internal {

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
