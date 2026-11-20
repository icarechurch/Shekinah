#include "shekinah/ui/reflow_text_edit.hpp"

#include <QKeyEvent>
#include <QTextCharFormat>

#include <algorithm>

namespace shekinah::ui {

ReflowTextEdit::ReflowTextEdit(QWidget* parent) : QPlainTextEdit(parent) {
    setObjectName(QStringLiteral("reflow-text-edit"));
    setPlaceholderText(QStringLiteral("Enter lyric text. Ctrl+Enter creates a slide split."));
}

void ReflowTextEdit::set_text(std::string text) {
    set_document(application::SongReflowDocument{std::move(text), {}});
}

void ReflowTextEdit::set_document(application::SongReflowDocument document) {
    document_ = std::move(document);
    setPlainText(QString::fromUtf8(document_.text().data(), static_cast<qsizetype>(document_.text().size())));
    rebuild_markers();
}

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

domain::Status ReflowTextEdit::insert_slide_break() {
    const auto result = document_.insert_slide_break(cursor_utf8_offset());
    if (!result) {
        return result;
    }
    rebuild_markers();
    if (document_changed_handler_) {
        document_changed_handler_();
    }
    return result;
}

domain::Status ReflowTextEdit::remove_adjacent_slide_break() {
    const auto offset = cursor_utf8_offset();
    const auto& breaks = document_.breaks();
    const auto adjacent = std::find_if(breaks.begin(), breaks.end(), [&](const auto& slide_break) {
        return slide_break.utf8_offset == offset ||
               (offset > 0 && slide_break.utf8_offset + 1 == offset);
    });
    if (adjacent == breaks.end()) {
        return domain::Status::fail(
            {domain::ErrorCode::not_found, "No adjacent Reflow break exists"});
    }
    const auto result = document_.remove_slide_break(adjacent->utf8_offset);
    if (result) {
        rebuild_markers();
        if (document_changed_handler_) {
            document_changed_handler_();
        }
    }
    return result;
}

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

int ReflowTextEdit::qt_position_for_utf8_offset(std::size_t offset) const {
    const auto text = toPlainText();
    for (int position = 0; position <= text.size(); ++position) {
        if (static_cast<std::size_t>(text.left(position).toUtf8().size()) >= offset) {
            return position;
        }
    }
    return text.size();
}

void ReflowTextEdit::rebuild_markers() {
    break_markers_.clear();
    for (const auto& slide_break : document_.breaks()) {
        QTextCursor cursor = textCursor();
        const auto position = qt_position_for_utf8_offset(slide_break.utf8_offset);
        cursor.setPosition(position);
        cursor.setPosition(std::min(position + 1, static_cast<int>(toPlainText().size())), QTextCursor::KeepAnchor);
        QTextEdit::ExtraSelection marker;
        marker.cursor = cursor;
        marker.format.setBackground(QColor{QStringLiteral("#f4c95d")});
        marker.format.setProperty(QTextFormat::FullWidthSelection, true);
        break_markers_.push_back(marker);
    }
    setExtraSelections(break_markers_);
}

}  // namespace shekinah::ui
