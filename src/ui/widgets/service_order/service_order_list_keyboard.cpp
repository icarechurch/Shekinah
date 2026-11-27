#include "service_order_list.hpp"

#include <QKeyEvent>

namespace shekinah::ui::service_order_internal {

void ServiceOrderListWidget::keyPressEvent(QKeyEvent* event) {
    if ((event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace) && key_remove_) {
        key_remove_();
        event->accept();
        return;
    }

    if (event->modifiers().testFlag(Qt::AltModifier) && event->key() == Qt::Key_Up && key_move_delta_) {
        key_move_delta_(-1);
        event->accept();
        return;
    }

    if (event->modifiers().testFlag(Qt::AltModifier) && event->key() == Qt::Key_Down && key_move_delta_) {
        key_move_delta_(1);
        event->accept();
        return;
    }

    if ((event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) &&
        event->modifiers().testFlag(Qt::ControlModifier) && key_edit_) {
        key_edit_();
        event->accept();
        return;
    }

    QListWidget::keyPressEvent(event);
}

}  // namespace shekinah::ui::service_order_internal
