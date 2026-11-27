#include "song_asset_coordinator.hpp"

#include "content_persistence_coordinator.hpp"
#include "shekinah/application/song_backgrounds.hpp"
#include "shekinah/storage/asset_repository.hpp"
#include "shekinah/storage/project_session.hpp"
#include "shekinah/storage/song_repository.hpp"

#include <algorithm>
#include <variant>

namespace shekinah::ui {
namespace {

bool is_song(const domain::ContentRef& content) noexcept {
    return content.kind == domain::ContentKind::song;
}

}  // namespace

SongAssetCoordinator::SongAssetCoordinator(
    storage::ProjectSession& session, ContentPersistenceCoordinator& persistence) noexcept
    : session_(session), persistence_(persistence) {}

domain::Status SongAssetCoordinator::assign_default(
    const domain::ContentRef& content, const domain::AssetId& asset) const {
    if (!session_.assets() || !session_.songs() || !is_song(content)) {
        return domain::Status::fail(
            {domain::ErrorCode::storage_failure, "Song background assignment is unavailable"});
    }
    const auto asset_record = session_.assets()->load_asset(asset);
    if (!asset_record) {
        return domain::Status::fail(asset_record.error());
    }
    auto song = session_.songs()->load_song(std::get<domain::SongId>(content.id));
    if (!song) {
        return domain::Status::fail(song.error());
    }
    application::set_default_background(
        *song, domain::BackgroundRef{asset_record->id, domain::FitMode::cover});
    const auto saved = persistence_.save_song(*song);
    return saved ? domain::success() : domain::Status::fail(saved.error());
}

domain::Status SongAssetCoordinator::assign_from_slide(
    const domain::ContentRef& content,
    const domain::CueId& cue,
    const domain::AssetId& asset) const {
    if (!session_.assets() || !session_.songs() || !is_song(content)) {
        return domain::Status::fail(
            {domain::ErrorCode::storage_failure, "Song background assignment is unavailable"});
    }
    const auto asset_record = session_.assets()->load_asset(asset);
    if (!asset_record) {
        return domain::Status::fail(asset_record.error());
    }
    auto song = session_.songs()->load_song(std::get<domain::SongId>(content.id));
    if (!song) {
        return domain::Status::fail(song.error());
    }
    const auto changed = application::apply_background_from_slide(
        *song, cue, domain::BackgroundRef{asset_record->id, domain::FitMode::cover});
    if (!changed) {
        return changed;
    }
    const auto saved = persistence_.save_song(*song);
    return saved ? domain::success() : domain::Status::fail(saved.error());
}

domain::Status SongAssetCoordinator::remove(
    const domain::ContentRef& content, const domain::AssetId& asset) const {
    if (!session_.songs() || !is_song(content)) {
        return domain::Status::fail(
            {domain::ErrorCode::storage_failure, "Song asset removal is unavailable"});
    }
    auto song = session_.songs()->load_song(std::get<domain::SongId>(content.id));
    if (!song) {
        return domain::Status::fail(song.error());
    }
    application::remove_asset_from_song(*song, asset);
    const auto saved = persistence_.save_song(*song);
    return saved ? domain::success() : domain::Status::fail(saved.error());
}

domain::Status SongAssetCoordinator::set_slide_background(
    const std::optional<domain::ServiceOrderItem>& selected_item,
    const domain::CueId& cue,
    const domain::AssetId& asset) const {
    if (!session_.assets() || !session_.songs()) {
        return domain::Status::fail(
            {domain::ErrorCode::storage_failure, "Open a project before setting a background"});
    }
    if (!selected_item.has_value() || !is_song(selected_item->content)) {
        return domain::Status::fail(
            {domain::ErrorCode::invalid_input, "Only selected Song cues accept editable backgrounds"});
    }
    const auto asset_record = session_.assets()->load_asset(asset);
    if (!asset_record) {
        return domain::Status::fail(asset_record.error());
    }
    auto song = session_.songs()->load_song(
        std::get<domain::SongId>(selected_item->content.id));
    if (!song) {
        return domain::Status::fail(song.error());
    }
    const auto slide = std::find_if(song->slides.begin(), song->slides.end(), [&](const auto& candidate) {
        return candidate.id == cue;
    });
    if (slide == song->slides.end()) {
        return domain::Status::fail({domain::ErrorCode::not_found, "Song cue was not found"});
    }
    slide->background = domain::BackgroundRef{asset_record->id, domain::FitMode::cover};
    const auto saved = persistence_.save_song(*song);
    return saved ? domain::success() : domain::Status::fail(saved.error());
}

}  // namespace shekinah::ui
