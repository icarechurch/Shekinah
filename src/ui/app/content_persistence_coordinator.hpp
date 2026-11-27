#pragma once

#include "shekinah/domain/model.hpp"
#include "shekinah/domain/result.hpp"

namespace shekinah::application {
class PresentationService;
}

namespace shekinah::storage {
class ProjectSession;
}

namespace shekinah::ui {

class ContentPersistenceCoordinator final {
public:
    ContentPersistenceCoordinator(storage::ProjectSession& session,
                                  application::PresentationService& presentation) noexcept;

    domain::Result<domain::Song> save_song(const domain::Song& song);
    domain::Result<domain::SermonDeck> save_sermon(const domain::SermonDeck& sermon);

private:
    storage::ProjectSession& session_;
    application::PresentationService& presentation_;
};

}  // namespace shekinah::ui
