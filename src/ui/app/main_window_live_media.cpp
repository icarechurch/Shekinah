#include "main_window_live_media.hpp"

#include "shekinah/storage/asset_repository.hpp"
#include "shekinah/storage/project_session.hpp"
#include "shekinah/storage/sermon_repository.hpp"
#include "shekinah/storage/song_repository.hpp"

namespace shekinah::ui::main_window_internal {

LiveMediaCoordinator::LiveMediaCoordinator(storage::ProjectSession& session) noexcept
    : session_(session) {}

bool LiveMediaCoordinator::ready() const noexcept {
    return session_.songs() != nullptr && session_.sermons() != nullptr &&
           session_.assets() != nullptr;
}

std::optional<LiveMediaSnapshot> LiveMediaCoordinator::resolve(
    const domain::ProgramState& program) const {
    if (program.mode != domain::ProgramMode::cue || !program.live_cue.has_value()) {
        return std::nullopt;
    }

    const auto* songs = session_.songs();
    const auto* sermons = session_.sermons();
    const auto* assets = session_.assets();
    const auto& cue = *program.live_cue;
    LiveMediaSnapshot snapshot;
    if (cue.content.kind == domain::ContentKind::song) {
        const auto song = songs->load_song(std::get<domain::SongId>(cue.content.id));
        if (song) {
            snapshot.presentation = application::build_live_song_presentation(*song, cue.cue);
        }
    } else {
        const auto deck = sermons->load_deck(std::get<domain::SermonId>(cue.content.id));
        if (deck) {
            snapshot.presentation = application::build_live_sermon_presentation(*deck, cue.cue);
        }
    }

    if (snapshot.presentation.background_asset.has_value()) {
        const auto asset = assets->load_asset(*snapshot.presentation.background_asset);
        if (asset) {
            snapshot.asset = *asset;
            const auto asset_path =
                assets->resolve_asset_path(*snapshot.presentation.background_asset);
            if (asset_path) {
                snapshot.asset_path = *asset_path;
            }
        }
    }
    return snapshot;
}

}  // namespace shekinah::ui::main_window_internal
