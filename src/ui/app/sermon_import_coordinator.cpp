#include "sermon_import_coordinator.hpp"

#include "shekinah/storage/project_session.hpp"
#include "shekinah/storage/sermon_deck_importer.hpp"

#include <utility>

namespace shekinah::ui {

SermonImportCoordinator::SermonImportCoordinator(storage::ProjectSession& session) noexcept
    : session_(session) {}

domain::Result<domain::SermonDeck> SermonImportCoordinator::import_files(
    domain::SermonId id,
    std::string title,
    std::vector<std::filesystem::path> sources,
    bool preserve_order) const {
    auto* importer = session_.sermon_importer();
    if (!session_.is_open() || importer == nullptr) {
        return domain::Result<domain::SermonDeck>::fail(
            {domain::ErrorCode::storage_failure, "Open a project before importing sermons"});
    }
    return importer->import_files(
        std::move(id), std::move(title), std::move(sources), preserve_order);
}

}  // namespace shekinah::ui
