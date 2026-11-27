#include "library_catalog_coordinator.hpp"

#include "asset_catalog_coordinator.hpp"
#include "shekinah/storage/project_session.hpp"
#include "shekinah/storage/sermon_repository.hpp"
#include "shekinah/storage/song_repository.hpp"
#include "shekinah/ui/library_dock_widget.hpp"

namespace shekinah::ui {

LibraryCatalogCoordinator::LibraryCatalogCoordinator(
    storage::ProjectSession& session, AssetCatalogCoordinator& assets) noexcept
    : session_(session), assets_(assets) {}

void LibraryCatalogCoordinator::refresh(LibraryDockWidget& library) {
    library.clear_contents();

    auto* songs = session_.songs();
    auto* sermons = session_.sermons();
    if (!assets_.available() || songs == nullptr || sermons == nullptr) {
        return;
    }

    library.set_asset_root(session_.root());
    const auto asset_records = assets_.list();
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
