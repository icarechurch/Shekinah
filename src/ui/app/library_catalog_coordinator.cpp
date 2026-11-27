#include "library_catalog_coordinator.hpp"

#include "shekinah/storage/asset_repository.hpp"
#include "shekinah/storage/project_session.hpp"
#include "shekinah/storage/sermon_repository.hpp"
#include "shekinah/storage/song_repository.hpp"
#include "shekinah/ui/library_dock_widget.hpp"

namespace shekinah::ui {

LibraryCatalogCoordinator::LibraryCatalogCoordinator(storage::ProjectSession& session) noexcept
    : session_(session) {}

void LibraryCatalogCoordinator::refresh(LibraryDockWidget& library) {
    library.clear_contents();

    auto* assets = session_.assets();
    auto* songs = session_.songs();
    auto* sermons = session_.sermons();
    if (assets == nullptr || songs == nullptr || sermons == nullptr) {
        return;
    }

    library.set_asset_root(session_.root());
    const auto asset_records = assets->list_assets();
    if (asset_records) {
        for (const auto& asset : *asset_records) {
            library.add_asset(asset);
        }
    }

    const auto song_records = songs->list_songs();
    if (song_records) {
        for (const auto& song : *song_records) {
            library.add_song(song);
        }
    }

    const auto sermon_records = sermons->list_decks();
    if (sermon_records) {
        for (const auto& sermon : *sermon_records) {
            library.add_sermon(sermon);
        }
    }
}

}  // namespace shekinah::ui
