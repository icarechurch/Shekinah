#pragma once

#include "shekinah/domain/model.hpp"
#include "shekinah/domain/result.hpp"

#include <optional>

namespace shekinah::storage {
class ProjectSession;
}

namespace shekinah::ui {
class ContentPersistenceCoordinator;

class SongAssetCoordinator final {
public:
    SongAssetCoordinator(storage::ProjectSession& session,
                         ContentPersistenceCoordinator& persistence) noexcept;

    [[nodiscard]] domain::Status assign_default(const domain::ContentRef& content,
                                                const domain::AssetId& asset) const;
    [[nodiscard]] domain::Status assign_from_slide(const domain::ContentRef& content,
                                                   const domain::CueId& cue,
                                                   const domain::AssetId& asset) const;
    [[nodiscard]] domain::Status remove(const domain::ContentRef& content,
                                        const domain::AssetId& asset) const;
    [[nodiscard]] domain::Status set_slide_background(
        const std::optional<domain::ServiceOrderItem>& selected_item,
        const domain::CueId& cue,
        const domain::AssetId& asset) const;

private:
    storage::ProjectSession& session_;
    ContentPersistenceCoordinator& persistence_;
};

}  // namespace shekinah::ui
