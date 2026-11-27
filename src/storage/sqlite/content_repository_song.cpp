#include "shekinah/storage/content_repository.hpp"

#include "internal/sqlite_song_repository.hpp"

namespace shekinah::storage {

domain::Result<domain::Song> SqliteContentRepository::save_song(const domain::Song& song) {
    return songs_->save_song(song);
}

domain::Result<domain::Song> SqliteContentRepository::load_song(const domain::SongId& id) const {
    return songs_->load_song(id);
}

domain::Result<application::SongReflowDocument> SqliteContentRepository::load_song_reflow(
    const domain::SongId& id) const {
    return songs_->load_song_reflow(id);
}

domain::Result<std::vector<domain::Song>> SqliteContentRepository::list_songs() const {
    return songs_->list_songs();
}

domain::Result<std::size_t> SqliteContentRepository::song_count() const {
    return songs_->song_count();
}

}  // namespace shekinah::storage
