#include "asset_slide_insertion_coordinator.hpp"

#include "content_persistence_coordinator.hpp"
#include "shekinah/application/slide_insertion.hpp"
#include "shekinah/storage/asset_repository.hpp"
#include "shekinah/storage/project_session.hpp"
#include "shekinah/storage/sermon_repository.hpp"
#include "shekinah/storage/song_repository.hpp"

#include <variant>

namespace shekinah::ui {

AssetSlideInsertionCoordinator::AssetSlideInsertionCoordinator(
    storage::ProjectSession& session, ContentPersistenceCoordinator& persistence) noexcept
    : session_(session), persistence_(persistence) {}

std::optional<int> AssetSlideInsertionCoordinator::insert(
    std::size_t index, const domain::ContentRef& content, const domain::AssetId& asset) const {
    if (session_.assets() == nullptr) {
        return std::nullopt;
    }
    const auto asset_record = session_.assets()->load_asset(asset);
    if (!asset_record) {
        return std::nullopt;
    }

    if (content.kind == domain::ContentKind::song) {
        if (session_.songs() == nullptr) {
            return std::nullopt;
        }
        auto song = session_.songs()->load_song(std::get<domain::SongId>(content.id));
        if (!song) {
            return std::nullopt;
        }
        const auto inserted = application::insert_asset_slide(
            *song, index, domain::BackgroundRef{asset_record->id, domain::FitMode::cover});
        if (!inserted) {
            return std::nullopt;
        }
        const auto saved = persistence_.save_song(*song);
        if (!saved) {
            return std::nullopt;
        }
        return static_cast<int>(saved->slides.size());
    }

    if (session_.sermons() == nullptr) {
        return std::nullopt;
    }
    auto sermon = session_.sermons()->load_deck(std::get<domain::SermonId>(content.id));
    if (!sermon) {
        return std::nullopt;
    }
    const auto inserted = application::insert_asset_slide(*sermon, index, asset_record->id);
    if (!inserted) {
        return std::nullopt;
    }
    const auto saved = persistence_.save_sermon(*sermon);
    if (!saved) {
        return std::nullopt;
    }
    return static_cast<int>(saved->slides.size());
}

}  // namespace shekinah::ui
