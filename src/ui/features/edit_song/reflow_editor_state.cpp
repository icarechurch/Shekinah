#include "shekinah/ui/reflow_editor_dialog.hpp"

#include "shekinah/ui/reflow_text_edit.hpp"

#include <QLineEdit>

#include <utility>

namespace shekinah::ui {

void ReflowEditorDialog::set_text(std::string text) {
    editor_->set_text(std::move(text));
    refresh_thumbnails();
}

void ReflowEditorDialog::set_reflow_document(application::SongReflowDocument document) {
    editor_->set_document(std::move(document));
    refresh_thumbnails();
}

void ReflowEditorDialog::load_song(domain::SongId id) { song_id_ = std::move(id); }

void ReflowEditorDialog::set_title(QString title) { title_->setText(std::move(title)); }

domain::Status ReflowEditorDialog::save_song() const {
    if (song_id_.empty()) {
        return domain::Status::fail(
            {domain::ErrorCode::invalid_input, "A Song needs an id before it can be saved"});
    }
    if (title_->text().trimmed().isEmpty()) {
        return domain::Status::fail(
            {domain::ErrorCode::invalid_input, "A Song needs a title before it can be saved"});
    }
    if (editor_->plain_text_utf8().empty()) {
        return domain::Status::fail(
            {domain::ErrorCode::invalid_input, "A Song cannot be saved without lyric text"});
    }
    return domain::success();
}

domain::Result<domain::Song> ReflowEditorDialog::draft_song() const {
    const auto status = save_song();
    if (!status) {
        return domain::Result<domain::Song>::fail(status.error());
    }
    return domain::Result<domain::Song>::ok(domain::Song{
        song_id_,
        title_->text().trimmed().toStdString(),
        std::nullopt,
        editor_->reflow_document().to_slides(),
    });
}

const application::SongReflowDocument& ReflowEditorDialog::reflow_document() const noexcept {
    return editor_->reflow_document();
}

}  // namespace shekinah::ui
