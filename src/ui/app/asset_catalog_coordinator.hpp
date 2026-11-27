#pragma once

#include "shekinah/domain/model.hpp"
#include "shekinah/domain/result.hpp"

#include <optional>
#include <string>
#include <vector>

namespace shekinah::storage {
class ProjectSession;
}

namespace shekinah::ui {

class AssetCatalogCoordinator final {
public:
    explicit AssetCatalogCoordinator(storage::ProjectSession& session) noexcept;

    [[nodiscard]] bool available() const noexcept;
    [[nodiscard]] domain::Result<std::vector<domain::AssetRecord>> list() const;
    [[nodiscard]] domain::Result<domain::AssetRecord> load(
        const domain::AssetId& id) const;
    [[nodiscard]] std::optional<std::string> filename(
        const domain::AssetId& id) const;

private:
    storage::ProjectSession& session_;
};

}  // namespace shekinah::ui
