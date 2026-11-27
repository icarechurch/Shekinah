#include "shekinah/ui/main_window.hpp"

#include "shekinah/storage/asset_repository.hpp"
#include "shekinah/storage/sermon_repository.hpp"
#include "shekinah/storage/song_repository.hpp"
#include "shekinah/ui/library_dock_widget.hpp"

namespace shekinah::ui {

void ShekinahWindow::refresh_library() {
    library_dock_->clear_contents();
    if (!assets() || !songs() || !sermons()) {
        return;
    }

    library_dock_->set_asset_root(session_.root());
    const auto asset_records = assets()->list_assets();
    if (asset_records) {
        for (const auto& asset : *asset_records) {
            library_dock_->add_asset(asset);
        }
    }
    const auto song_records = songs()->list_songs();
    if (song_records) {
        for (const auto& song : *song_records) {
            library_dock_->add_song(song);
        }
    }
    const auto sermon_records = sermons()->list_decks();
    if (sermon_records) {
        for (const auto& sermon : *sermon_records) {
            library_dock_->add_sermon(sermon);
        }
    }
}

}  // namespace shekinah::ui
