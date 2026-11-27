#pragma once

#include "shekinah/domain/model.hpp"
#include "shekinah/domain/result.hpp"

#include <filesystem>

namespace shekinah::storage {
class ProjectSession;
}

namespace shekinah::ui {
class ContentPersistenceCoordinator;

class SongImportCoordinator final {
public:
    SongImportCoordinator(storage::ProjectSession& session,
                          ContentPersistenceCoordinator& persistence) noexcept;

    [[nodiscard]] domain::Result<domain::Song> import_file(
        domain::SongId id, const std::filesystem::path& source) const;

private:
    storage::ProjectSession& session_;
    ContentPersistenceCoordinator& persistence_;
};

}  // namespace shekinah::ui
