#include "shekinah/ui/main_window.hpp"

#include "shekinah/storage/content_repository.hpp"
#include "shekinah/ui/library_dock_widget.hpp"

namespace shekinah::ui {

void ShekinahWindow::refresh_library() {
    library_dock_->clear_contents();
    if (!repository()) {
        return;
    }

    library_dock_->set_asset_root(session_.root());
    const auto assets = repository()->list_assets();
    if (assets) {
        for (const auto& asset : *assets) {
            library_dock_->add_asset(asset);
        }
    }
    const auto songs = repository()->list_songs();
    if (songs) {
        for (const auto& song : *songs) {
            library_dock_->add_song(song);
        }
    }
    const auto sermons = repository()->list_decks();
    if (sermons) {
        for (const auto& sermon : *sermons) {
            library_dock_->add_sermon(sermon);
        }
    }
}

}  // namespace shekinah::ui
