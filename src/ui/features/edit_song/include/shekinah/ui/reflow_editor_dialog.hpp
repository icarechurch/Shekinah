#pragma once

#include "shekinah/application/song_reflow.hpp"
#include "shekinah/domain/result.hpp"

#include <QDialog>

class QListWidget;
class QLineEdit;

namespace shekinah::ui {

class ReflowTextEdit;

class ReflowEditorDialog final : public QDialog {
public:
    explicit ReflowEditorDialog(QWidget* parent = nullptr);

    void set_text(std::string text);
    void load_song(domain::SongId id);
    void set_title(QString title);
    domain::Status save_song() const;
    domain::Result<domain::Song> draft_song() const;
    [[nodiscard]] ReflowTextEdit* editor() const noexcept { return editor_; }
    [[nodiscard]] const application::SongReflowDocument& reflow_document() const noexcept;
    [[nodiscard]] int thumbnail_count() const noexcept;

private:
    void refresh_thumbnails();

    domain::SongId song_id_;
    QLineEdit* title_{nullptr};
    ReflowTextEdit* editor_{nullptr};
    QListWidget* thumbnails_{nullptr};
};

}  // namespace shekinah::ui
