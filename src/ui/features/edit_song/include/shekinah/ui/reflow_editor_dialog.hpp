#pragma once

#include "shekinah/application/song_reflow.hpp"

#include <QDialog>

class QListWidget;

namespace shekinah::ui {

class ReflowTextEdit;

class ReflowEditorDialog final : public QDialog {
public:
    explicit ReflowEditorDialog(QWidget* parent = nullptr);

    void set_text(std::string text);
    void load_song(domain::SongId id);
    domain::Status save_song() const;
    [[nodiscard]] ReflowTextEdit* editor() const noexcept { return editor_; }
    [[nodiscard]] const application::SongReflowDocument& reflow_document() const noexcept;
    [[nodiscard]] int thumbnail_count() const noexcept;

private:
    void refresh_thumbnails();

    domain::SongId song_id_;
    ReflowTextEdit* editor_{nullptr};
    QListWidget* thumbnails_{nullptr};
};

}  // namespace shekinah::ui
