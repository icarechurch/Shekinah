#pragma once

#include "shekinah/domain/model.hpp"

#include <cstddef>
#include <optional>

namespace shekinah::storage {
class ProjectSession;
}

namespace shekinah::ui {
class ContentPersistenceCoordinator;

class AssetSlideInsertionCoordinator final {
public:
    AssetSlideInsertionCoordinator(storage::ProjectSession& session,
                                   ContentPersistenceCoordinator& persistence) noexcept;

    [[nodiscard]] std::optional<int> insert(std::size_t index,
                                            const domain::ContentRef& content,
                                            const domain::AssetId& asset) const;

private:
    storage::ProjectSession& session_;
    ContentPersistenceCoordinator& persistence_;
};

}  // namespace shekinah::ui
