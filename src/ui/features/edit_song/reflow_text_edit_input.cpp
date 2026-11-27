#include "shekinah/ui/reflow_text_edit.hpp"

#include <QKeyEvent>

namespace shekinah::ui {

void ReflowTextEdit::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Return && event->modifiers().testFlag(Qt::ControlModifier)) {
        insert_slide_break();
        event->accept();
        return;
    }
    if (event->key() == Qt::Key_Backspace && textCursor().hasSelection() == false) {
        const auto result = remove_adjacent_slide_break();
        if (result) {
            event->accept();
            return;
        }
    }
    QPlainTextEdit::keyPressEvent(event);
}

}  // namespace shekinah::ui
