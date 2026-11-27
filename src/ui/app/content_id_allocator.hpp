#pragma once

#include "shekinah/domain/model.hpp"

#include <string>

namespace shekinah::storage {
class ProjectSession;
}

namespace shekinah::ui {

class ContentIdAllocator final {
public:
    explicit ContentIdAllocator(storage::ProjectSession& session) noexcept;

    [[nodiscard]] domain::SongId allocate_song_id();
    [[nodiscard]] domain::SermonId allocate_sermon_id();
    void reset() noexcept;

private:
    storage::ProjectSession& session_;
    int next_content_id_{1};
};

}  // namespace shekinah::ui
