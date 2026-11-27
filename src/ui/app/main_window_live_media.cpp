#include "main_window_live_media.hpp"

#include <QString>

namespace shekinah::ui::main_window_internal {

std::optional<LiveMediaSnapshot> resolve_live_media(
    const storage::SqliteContentRepository& repository,
    const domain::ProgramState& program,
    const std::filesystem::path& project_root) {
    if (program.mode != domain::ProgramMode::cue || !program.live_cue.has_value()) {
        return std::nullopt;
    }

    const auto& cue = *program.live_cue;
    LiveMediaSnapshot snapshot;
    if (cue.content.kind == domain::ContentKind::song) {
        const auto song = repository.load_song(std::get<domain::SongId>(cue.content.id));
        if (song) {
            snapshot.presentation = application::build_live_song_presentation(*song, cue.cue);
        }
    } else {
        const auto deck = repository.load_deck(std::get<domain::SermonId>(cue.content.id));
        if (deck) {
            snapshot.presentation = application::build_live_sermon_presentation(*deck, cue.cue);
        }
    }

    if (snapshot.presentation.background_asset.has_value()) {
        const auto asset = repository.load_asset(*snapshot.presentation.background_asset);
        if (asset) {
            snapshot.asset = *asset;
            snapshot.asset_path = project_root / snapshot.asset->relative_path;
        }
    }
    return snapshot;
}

LiveSlideDetails make_live_details(
    const application::LiveSlidePresentation& presentation,
    const std::optional<domain::AssetRecord>& asset) {
    LiveSlideDetails details{};
    details.item_title = QString::fromStdString(presentation.item_title);
    details.section = QString::fromStdString(presentation.section);
    details.slide_text = QString::fromStdString(presentation.slide_text);
    details.next_section = QString::fromStdString(presentation.next_section);
    details.next_summary = QString::fromStdString(presentation.next_summary);
    details.slide_index = presentation.slide_index;
    details.total_slides = presentation.total_slides;
    if (presentation.background_asset.has_value()) {
        details.fit_mode = presentation.background_fit;
    }
    if (asset.has_value()) {
        const auto filename = std::filesystem::path{asset->relative_path}.filename().string();
        details.has_background = true;
        details.is_video = (asset->kind == domain::AssetKind::video);
        details.asset_name = QString::fromStdString(filename.empty() ? asset->relative_path : filename);
    }
    return details;
}

}  // namespace shekinah::ui::main_window_internal
