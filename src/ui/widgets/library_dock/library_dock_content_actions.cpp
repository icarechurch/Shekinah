#include "shekinah/ui/library_dock_widget.hpp"

#include "library_components.hpp"

#include <QListWidget>

namespace shekinah::ui {
using namespace library_internal;

void LibraryDockWidget::activate_song_row(int row) {
    if (row >= 0 && row < songs_->count()) {
        const auto* item = songs_->item(row);
        if (add_content_handler_ && item != nullptr) {
            add_content_handler_(
                domain::ContentRef::song(item->data(kContentIdRole).toString().toStdString()));
        }
    }
}

void LibraryDockWidget::edit_song_row(int row) {
    if (row >= 0 && row < songs_->count()) {
        const auto* item = songs_->item(row);
        if (edit_song_handler_ && item != nullptr) {
            edit_song_handler_(domain::SongId{item->data(kContentIdRole).toString().toStdString()});
        }
    }
}

void LibraryDockWidget::activate_sermon_row(int row) {
    if (row >= 0 && row < sermons_->count()) {
        const auto* item = sermons_->item(row);
        if (add_content_handler_ && item != nullptr) {
            add_content_handler_(
                domain::ContentRef::sermon(item->data(kContentIdRole).toString().toStdString()));
        }
    }
}

void LibraryDockWidget::edit_sermon_row(int row) {
    if (row >= 0 && row < sermons_->count()) {
        const auto* item = sermons_->item(row);
        if (edit_sermon_handler_ && item != nullptr) {
            edit_sermon_handler_(
                domain::SermonId{item->data(kContentIdRole).toString().toStdString()});
        }
    }
}

}  // namespace shekinah::ui
