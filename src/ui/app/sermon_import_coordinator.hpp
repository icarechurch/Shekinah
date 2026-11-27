#pragma once

#include "shekinah/domain/model.hpp"
#include "shekinah/domain/result.hpp"

#include <filesystem>
#include <string>
#include <vector>

namespace shekinah::storage {
class ProjectSession;
}

namespace shekinah::ui {

class SermonImportCoordinator final {
public:
    explicit SermonImportCoordinator(storage::ProjectSession& session) noexcept;

    [[nodiscard]] domain::Result<domain::SermonDeck> import_files(
        domain::SermonId id,
        std::string title,
        std::vector<std::filesystem::path> sources,
        bool preserve_order) const;

private:
    storage::ProjectSession& session_;
};

}  // namespace shekinah::ui
