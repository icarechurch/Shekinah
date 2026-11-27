#pragma once

#include "shekinah/domain/model.hpp"
#include "shekinah/domain/result.hpp"

#include <filesystem>

namespace shekinah::storage {
class ProjectSession;
}

namespace shekinah::ui {

class AssetImportCoordinator final {
public:
    explicit AssetImportCoordinator(storage::ProjectSession& session) noexcept;

    [[nodiscard]] domain::Result<domain::AssetRecord> import_file(
        const std::filesystem::path& source) const;

private:
    storage::ProjectSession& session_;
};

}  // namespace shekinah::ui
