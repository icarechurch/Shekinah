#include "shekinah/ui/reflow_text_edit.hpp"

namespace shekinah::ui {

void ReflowTextEdit::set_cursor_utf8_offset(std::size_t offset) {
    auto cursor = textCursor();
    cursor.setPosition(qt_position_for_utf8_offset(offset));
    setTextCursor(cursor);
}

std::size_t ReflowTextEdit::cursor_utf8_offset() const {
    const auto position = textCursor().position();
    return static_cast<std::size_t>(toPlainText().left(position).toUtf8().size());
}

std::string ReflowTextEdit::plain_text_utf8() const { return toPlainText().toUtf8().toStdString(); }

int ReflowTextEdit::qt_position_for_utf8_offset(std::size_t offset) const {
    const auto text = toPlainText();
    for (int position = 0; position <= text.size(); ++position) {
        if (static_cast<std::size_t>(text.left(position).toUtf8().size()) >= offset) {
            return position;
        }
    }
    return text.size();
}

}  // namespace shekinah::ui
