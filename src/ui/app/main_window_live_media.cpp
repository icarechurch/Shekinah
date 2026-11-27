#include "main_window_live_media.hpp"

namespace shekinah::ui::main_window_internal {

std::optional<LiveMediaSnapshot> resolve_live_media(
    const storage::SongRepository& songs,
    const storage::SermonRepository& sermons,
    const storage::AssetRepository& assets,
    const domain::ProgramState& program,
    const std::filesystem::path& project_root) {
    if (program.mode != domain::ProgramMode::cue || !program.live_cue.has_value()) {
        return std::nullopt;
    }

    const auto& cue = *program.live_cue;
    LiveMediaSnapshot snapshot;
    if (cue.content.kind == domain::ContentKind::song) {
        const auto song = songs.load_song(std::get<domain::SongId>(cue.content.id));
        if (song) {
            snapshot.presentation = application::build_live_song_presentation(*song, cue.cue);
        }
    } else {
        const auto deck = sermons.load_deck(std::get<domain::SermonId>(cue.content.id));
        if (deck) {
            snapshot.presentation = application::build_live_sermon_presentation(*deck, cue.cue);
        }
    }

    if (snapshot.presentation.background_asset.has_value()) {
        const auto asset = assets.load_asset(*snapshot.presentation.background_asset);
        if (asset) {
            snapshot.asset = *asset;
            snapshot.asset_path = project_root / snapshot.asset->relative_path;
        }
    }
    return snapshot;
}

}  // namespace shekinah::ui::main_window_internal
