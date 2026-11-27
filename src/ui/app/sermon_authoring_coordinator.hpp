#pragma once

#include "shekinah/domain/model.hpp"
#include "shekinah/domain/result.hpp"

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace shekinah::storage {
class ProjectSession;
}

namespace shekinah::ui {

class AssetImportCoordinator;
class ContentPersistenceCoordinator;

struct SermonEditState final {
    domain::SermonDeck existing;
    std::vector<std::filesystem::path> files;
};

class SermonAuthoringCoordinator final {
public:
    SermonAuthoringCoordinator(storage::ProjectSession& session,
                               AssetImportCoordinator& assets,
                               ContentPersistenceCoordinator& persistence) noexcept;

    [[nodiscard]] std::optional<SermonEditState> load_for_edit(
        const domain::SermonId& id) const;
    [[nodiscard]] domain::Result<domain::SermonDeck> save_edited_deck(
        domain::SermonDeck existing,
        std::string title,
        std::vector<std::filesystem::path> files) const;

private:
    storage::ProjectSession& session_;
    AssetImportCoordinator& assets_;
    ContentPersistenceCoordinator& persistence_;
};

}  // namespace shekinah::ui
