#pragma once

#include "shekinah/application/song_reflow.hpp"

#include <QPlainTextEdit>
#include <QTextEdit>

#include <functional>
#include <string>

namespace shekinah::ui {

class ReflowTextEdit final : public QPlainTextEdit {
public:
    explicit ReflowTextEdit(QWidget* parent = nullptr);

    void set_text(std::string text);
    void set_cursor_utf8_offset(std::size_t offset);
    [[nodiscard]] std::size_t cursor_utf8_offset() const;
    [[nodiscard]] std::string plain_text_utf8() const;
    [[nodiscard]] const application::SongReflowDocument& reflow_document() const noexcept {
        return document_;
    }
    [[nodiscard]] std::size_t visible_break_count() const noexcept { return break_markers_.size(); }

    void set_document_changed_handler(std::function<void()> handler) {
        document_changed_handler_ = std::move(handler);
    }
    domain::Status insert_slide_break();
    domain::Status remove_adjacent_slide_break();

protected:
    void keyPressEvent(QKeyEvent* event) override;

private:
    [[nodiscard]] int qt_position_for_utf8_offset(std::size_t offset) const;
    void rebuild_markers();

    application::SongReflowDocument document_{{}, {}};
    QList<QTextEdit::ExtraSelection> break_markers_;
    std::function<void()> document_changed_handler_;
};

}  // namespace shekinah::ui
