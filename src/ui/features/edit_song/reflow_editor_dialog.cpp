#include "shekinah/ui/reflow_editor_dialog.hpp"

#include "shekinah/ui/reflow_text_edit.hpp"

#include <QListWidget>
#include <QSplitter>
#include <QVBoxLayout>

namespace shekinah::ui {

ReflowEditorDialog::ReflowEditorDialog(QWidget* parent) : QDialog(parent) {
    setObjectName(QStringLiteral("reflow-editor-dialog"));
    setWindowTitle(QStringLiteral("Song Reflow"));
    auto* layout = new QVBoxLayout(this);
    auto* splitter = new QSplitter{Qt::Horizontal, this};
    editor_ = new ReflowTextEdit{splitter};
    thumbnails_ = new QListWidget{splitter};
    thumbnails_->setObjectName(QStringLiteral("reflow-thumbnails"));
    splitter->addWidget(editor_);
    splitter->addWidget(thumbnails_);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 0);
    layout->addWidget(splitter);
    editor_->set_document_changed_handler([this] { refresh_thumbnails(); });
    resize(920, 600);
}

void ReflowEditorDialog::set_text(std::string text) {
    editor_->set_text(std::move(text));
    refresh_thumbnails();
}

void ReflowEditorDialog::load_song(domain::SongId id) { song_id_ = std::move(id); }

domain::Status ReflowEditorDialog::save_song() const {
    if (editor_->plain_text_utf8().empty()) {
        return domain::Status::fail(
            {domain::ErrorCode::invalid_input, "A Song cannot be saved without lyric text"});
    }
    return domain::success();
}

const application::SongReflowDocument& ReflowEditorDialog::reflow_document() const noexcept {
    return editor_->reflow_document();
}

int ReflowEditorDialog::thumbnail_count() const noexcept { return thumbnails_->count(); }

void ReflowEditorDialog::refresh_thumbnails() {
    thumbnails_->clear();
    for (const auto& slide : editor_->reflow_document().to_slides()) {
        const auto section = slide.section.empty() ? "Slide" : slide.section;
        const auto summary = slide.lines.empty() ? "" : slide.lines.front();
        thumbnails_->addItem(QStringLiteral("%1 — %2")
                                 .arg(QString::fromStdString(section))
                                 .arg(QString::fromStdString(summary)));
    }
}

}  // namespace shekinah::ui
