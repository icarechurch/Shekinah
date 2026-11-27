#include "shekinah/ui/reflow_text_edit.hpp"

#include <algorithm>
#include <utility>

namespace shekinah::ui {

void ReflowTextEdit::set_text(std::string text) {
    set_document(application::SongReflowDocument{std::move(text), {}});
}

void ReflowTextEdit::set_document(application::SongReflowDocument document) {
    document_ = std::move(document);
    setPlainText(QString::fromUtf8(document_.text().data(), static_cast<qsizetype>(document_.text().size())));
    rebuild_markers();
}

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

}  // namespace shekinah::ui
