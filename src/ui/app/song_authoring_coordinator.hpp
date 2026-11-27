#pragma once

#include "shekinah/application/song_authoring.hpp"
#include "shekinah/domain/result.hpp"

#include <optional>

namespace shekinah::domain {
struct Song;
}

namespace shekinah::storage {
class ProjectSession;
}

namespace shekinah::ui {
class ContentPersistenceCoordinator;

struct SongEditState final {
    domain::Song existing;
    application::SongReflowDocument reflow;
};

class SongAuthoringCoordinator final {
public:
    SongAuthoringCoordinator(storage::ProjectSession& session,
                             ContentPersistenceCoordinator& persistence) noexcept;

    [[nodiscard]] std::optional<SongEditState> load_for_edit(const domain::SongId& id) const;
    [[nodiscard]] domain::Result<domain::Song> save_edited_song(
        domain::Song draft, const domain::Song& existing) const;

private:
    storage::ProjectSession& session_;
    ContentPersistenceCoordinator& persistence_;
};

}  // namespace shekinah::ui
