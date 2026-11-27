#include "content_persistence_coordinator.hpp"

#include "shekinah/application/presentation_service.hpp"
#include "shekinah/storage/project_session.hpp"
#include "shekinah/storage/sermon_repository.hpp"
#include "shekinah/storage/song_repository.hpp"

namespace shekinah::ui {

ContentPersistenceCoordinator::ContentPersistenceCoordinator(
    storage::ProjectSession& session, application::PresentationService& presentation) noexcept
    : session_(session), presentation_(presentation) {}

domain::Result<domain::Song> ContentPersistenceCoordinator::save_song(const domain::Song& song) {
    auto* songs = session_.songs();
    if (songs == nullptr) {
        return domain::Result<domain::Song>::fail(
            {domain::ErrorCode::storage_failure, "Open a project before saving songs"});
    }
    const auto saved = songs->save_song(song);
    if (saved) {
        presentation_.register_song(*saved);
    }
    return saved;
}

domain::Result<domain::SermonDeck> ContentPersistenceCoordinator::save_sermon(
    const domain::SermonDeck& sermon) {
    auto* sermons = session_.sermons();
    if (sermons == nullptr) {
        return domain::Result<domain::SermonDeck>::fail(
            {domain::ErrorCode::storage_failure, "Open a project before saving sermons"});
    }
    const auto saved = sermons->save_deck(sermon);
    if (saved) {
        presentation_.register_sermon(*saved);
    }
    return saved;
}

}  // namespace shekinah::ui
