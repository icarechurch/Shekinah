#include "cue_presentation_coordinator.hpp"

#include "shekinah/storage/asset_repository.hpp"
#include "shekinah/storage/project_session.hpp"
#include "shekinah/storage/sermon_repository.hpp"
#include "shekinah/storage/song_repository.hpp"

#include <filesystem>
#include <optional>

namespace shekinah::ui {

CuePresentationCoordinator::CuePresentationCoordinator(storage::ProjectSession& session) noexcept
    : session_(session) {}

bool CuePresentationCoordinator::ready() const noexcept {
    return session_.assets() != nullptr && session_.songs() != nullptr &&
           session_.sermons() != nullptr;
}

domain::Result<application::CuePresentationSet> CuePresentationCoordinator::load(
    const domain::ContentRef& content) const {
    if (!ready()) {
        return domain::Result<application::CuePresentationSet>::fail(
            {domain::ErrorCode::storage_failure, "Open a project before loading cues"});
    }

    const auto resolve_asset_path = [this](const domain::AssetId& asset_id)
        -> std::optional<std::filesystem::path> {
        const auto asset = session_.assets()->resolve_asset_path(asset_id);
        if (!asset) {
            return std::nullopt;
        }
        return *asset;
    };

    if (content.kind == domain::ContentKind::song) {
        const auto song = session_.songs()->load_song(std::get<domain::SongId>(content.id));
        if (!song) {
            return domain::Result<application::CuePresentationSet>::fail(song.error());
        }
        return application::build_song_cue_presentation(*song, resolve_asset_path);
    }

    const auto sermon = session_.sermons()->load_deck(std::get<domain::SermonId>(content.id));
    if (!sermon) {
        return domain::Result<application::CuePresentationSet>::fail(sermon.error());
    }
    return application::build_sermon_cue_presentation(*sermon, resolve_asset_path);
}

}  // namespace shekinah::ui
