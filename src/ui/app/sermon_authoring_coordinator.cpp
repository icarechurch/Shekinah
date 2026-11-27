#include "sermon_authoring_coordinator.hpp"

#include "asset_import_coordinator.hpp"
#include "content_persistence_coordinator.hpp"
#include "shekinah/storage/asset_repository.hpp"
#include "shekinah/storage/project_session.hpp"
#include "shekinah/storage/sermon_repository.hpp"

#include <cstddef>
#include <string>
#include <utility>

namespace shekinah::ui {
namespace {

domain::Result<domain::SermonDeck> authoring_unavailable() {
    return domain::Result<domain::SermonDeck>::fail(
        {domain::ErrorCode::storage_failure, "Sermon authoring is unavailable"});
}

}  // namespace

SermonAuthoringCoordinator::SermonAuthoringCoordinator(
    storage::ProjectSession& session,
    AssetImportCoordinator& assets,
    ContentPersistenceCoordinator& persistence) noexcept
    : session_(session), assets_(assets), persistence_(persistence) {}

std::optional<SermonEditState> SermonAuthoringCoordinator::load_for_edit(
    const domain::SermonId& id) const {
    if (!session_.is_open() || !session_.sermons() || !session_.assets()) {
        return std::nullopt;
    }
    const auto existing = session_.sermons()->load_deck(id);
    if (!existing) {
        return std::nullopt;
    }
    std::vector<std::filesystem::path> files;
    files.reserve(existing->slides.size());
    for (const auto& slide : existing->slides) {
        const auto path = session_.assets()->resolve_asset_path(slide.image);
        if (!path) {
            return std::nullopt;
        }
        files.push_back(*path);
    }
    return SermonEditState{*existing, std::move(files)};
}

domain::Result<domain::SermonDeck> SermonAuthoringCoordinator::save_edited_deck(
    domain::SermonDeck existing,
    std::string title,
    std::vector<std::filesystem::path> files) const {
    if (!session_.is_open() || !session_.sermons() || !session_.assets()) {
        return authoring_unavailable();
    }
    std::vector<domain::SermonSlide> slides;
    slides.reserve(files.size());
    for (std::size_t index = 0; index != files.size(); ++index) {
        const auto imported = assets_.import_file(files[index]);
        if (!imported) {
            return domain::Result<domain::SermonDeck>::fail(imported.error());
        }
        slides.push_back(domain::SermonSlide{
            domain::CueId{"slide-" + std::to_string(index)}, imported->id});
    }
    existing.title = std::move(title);
    existing.slides = std::move(slides);
    return persistence_.save_sermon(existing);
}

}  // namespace shekinah::ui
