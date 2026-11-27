#pragma once

#include "shekinah/domain/model.hpp"
#include "shekinah/domain/result.hpp"

namespace shekinah::storage {
class ProjectSession;
}

namespace shekinah::ui {
class ContentPersistenceCoordinator;

class SlideMutationCoordinator final {
public:
    SlideMutationCoordinator(storage::ProjectSession& session,
                             ContentPersistenceCoordinator& persistence) noexcept;

    [[nodiscard]] domain::Status erase(const domain::ContentRef& content,
                                       const domain::CueId& cue) const;
    [[nodiscard]] domain::Status duplicate(const domain::ContentRef& content,
                                           const domain::CueId& cue) const;

private:
    storage::ProjectSession& session_;
    ContentPersistenceCoordinator& persistence_;
};

}  // namespace shekinah::ui
