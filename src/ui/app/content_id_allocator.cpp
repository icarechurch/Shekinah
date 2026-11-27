#include "content_id_allocator.hpp"

#include "shekinah/storage/project_session.hpp"
#include "shekinah/storage/sermon_repository.hpp"
#include "shekinah/storage/song_repository.hpp"

#include <string>

namespace shekinah::ui {

ContentIdAllocator::ContentIdAllocator(storage::ProjectSession& session) noexcept
    : session_(session) {}

domain::SongId ContentIdAllocator::allocate_song_id() {
    for (;;) {
        const auto candidate = "song-" + std::to_string(next_content_id_++);
        if (!session_.songs() || !session_.songs()->load_song(domain::SongId{candidate})) {
            return domain::SongId{candidate};
        }
    }
}

domain::SermonId ContentIdAllocator::allocate_sermon_id() {
    for (;;) {
        const auto candidate = "sermon-" + std::to_string(next_content_id_++);
        if (!session_.sermons() ||
            !session_.sermons()->load_deck(domain::SermonId{candidate})) {
            return domain::SermonId{candidate};
        }
    }
}

void ContentIdAllocator::reset() noexcept {
    next_content_id_ = 1;
}

}  // namespace shekinah::ui
