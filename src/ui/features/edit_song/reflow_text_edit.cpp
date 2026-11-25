#include "shekinah/ui/reflow_text_edit.hpp"

#include "shekinah/ui/support/theme.hpp"

#include <QKeyEvent>
#include <QTextCharFormat>

#include <algorithm>

namespace shekinah::ui {

ReflowTextEdit::ReflowTextEdit(QWidget* parent) : QPlainTextEdit(parent) {
    setObjectName(QStringLiteral("reflow-text-edit"));
    setPlaceholderText(QStringLiteral("Enter lyric text. Ctrl+Enter creates a slide split."));
    QObject::connect(this, &QPlainTextEdit::textChanged, this, [this] {
        synchronize_document_text();
    });
    QObject::connect(&ThemeManager::instance(), &ThemeManager::theme_changed, this,
                     [this](ThemeId) { rebuild_markers(); });
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

void ReflowTextEdit::synchronize_document_text() {
    const auto current_text = plain_text_utf8();
    const auto& previous_text = document_.text();
    if (current_text == previous_text) {
        return;
    }

    std::size_t prefix = 0;
    const auto common_size = std::min(previous_text.size(), current_text.size());
    while (prefix < common_size && previous_text[prefix] == current_text[prefix]) {
        ++prefix;
    }

    std::size_t previous_suffix = previous_text.size();
    std::size_t current_suffix = current_text.size();
    while (previous_suffix > prefix && current_suffix > prefix &&
           previous_text[previous_suffix - 1] == current_text[current_suffix - 1]) {
        --previous_suffix;
        --current_suffix;
    }

    std::vector<application::SlideBreak> adjusted_breaks;
    adjusted_breaks.reserve(document_.breaks().size());
    for (const auto& slide_break : document_.breaks()) {
        std::size_t offset = slide_break.utf8_offset;
        if (offset < prefix) {
            // The break is before the edited range and remains unchanged.
        } else if (offset >= previous_suffix) {
            offset = current_suffix + (offset - previous_suffix);
        } else {
            // A break inside replaced text follows the newly typed text.
            offset = current_suffix;
        }
        offset = std::min(offset, current_text.size());
        adjusted_breaks.push_back(application::SlideBreak{offset, slide_break.section_label});
    }

    document_ = application::SongReflowDocument{current_text, std::move(adjusted_breaks)};
    rebuild_markers();
    if (document_changed_handler_) {
        document_changed_handler_();
    }
}

void ReflowTextEdit::rebuild_markers() {
    const auto& p = ThemeManager::instance().palette();
    break_markers_.clear();
    for (const auto& slide_break : document_.breaks()) {
        QTextCursor cursor = textCursor();
        const auto position = qt_position_for_utf8_offset(slide_break.utf8_offset);
        cursor.setPosition(position);
        cursor.setPosition(std::min(position + 1, static_cast<int>(toPlainText().size())), QTextCursor::KeepAnchor);
        QTextEdit::ExtraSelection marker;
        marker.cursor = cursor;
        marker.format.setBackground(p.reflow_break_bg);
        marker.format.setForeground(p.reflow_break_fg);
        marker.format.setProperty(QTextFormat::FullWidthSelection, true);
        break_markers_.push_back(marker);
    }
    setExtraSelections(break_markers_);
}

}  // namespace shekinah::ui
