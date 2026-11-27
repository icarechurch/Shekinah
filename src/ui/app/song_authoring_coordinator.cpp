#include "song_authoring_coordinator.hpp"

#include "content_persistence_coordinator.hpp"
#include "shekinah/storage/project_session.hpp"
#include "shekinah/storage/song_repository.hpp"

#include <cstddef>
#include <string>
#include <utility>

namespace shekinah::ui {
namespace {

domain::Result<domain::Song> authoring_unavailable() {
    return domain::Result<domain::Song>::fail(
        {domain::ErrorCode::storage_failure, "Song authoring is unavailable"});
}

}  // namespace

SongAuthoringCoordinator::SongAuthoringCoordinator(
    storage::ProjectSession& session, ContentPersistenceCoordinator& persistence) noexcept
    : session_(session), persistence_(persistence) {}

std::optional<SongEditState> SongAuthoringCoordinator::load_for_edit(
    const domain::SongId& id) const {
    if (!session_.songs()) {
        return std::nullopt;
    }
    const auto existing = session_.songs()->load_song(id);
    const auto reflow = session_.songs()->load_song_reflow(id);
    if (!existing || !reflow) {
        return std::nullopt;
    }
    return SongEditState{*existing, *reflow};
}

domain::Result<domain::Song> SongAuthoringCoordinator::save_edited_song(
    domain::Song draft, const domain::Song& existing) const {
    if (!session_.songs()) {
        return authoring_unavailable();
    }
    draft.default_background = existing.default_background;
    for (std::size_t index = 0; index != draft.slides.size(); ++index) {
        if (index < existing.slides.size()) {
            draft.slides[index].id = existing.slides[index].id;
            draft.slides[index].background = existing.slides[index].background;
        } else {
            draft.slides[index].id = domain::CueId{
                draft.id.value + "-slide-" + std::to_string(index)};
        }
    }
    return persistence_.save_song(draft);
}

}  // namespace shekinah::ui
