#include "shekinah/storage/content_repository.hpp"

#include "internal/sqlite_content_archive_repository.hpp"

namespace shekinah::storage {

domain::Status SqliteContentRepository::archive_song(const domain::SongId& id) {
    return archive_->archive_song(id);
}

domain::Status SqliteContentRepository::archive_deck(const domain::SermonId& id) {
    return archive_->archive_deck(id);
}

}  // namespace shekinah::storage
