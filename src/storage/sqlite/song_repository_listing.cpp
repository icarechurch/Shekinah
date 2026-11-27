#include "shekinah/storage/content_repository.hpp"

#include "internal/sqlite_support.hpp"

#include <utility>
#include <vector>

namespace shekinah::storage {
namespace {

using sqlite_internal::Statement;
using sqlite_internal::bind_text;

}  // namespace

domain::Result<std::vector<domain::Song>> SqliteContentRepository::list_songs() const {
    Statement statement{store_.native_handle(),
                        "SELECT id FROM songs WHERE archived=0 ORDER BY title COLLATE NOCASE ASC, id ASC;"};
    if (!statement.ready()) {
        return domain::Result<std::vector<domain::Song>>::fail(
            {domain::ErrorCode::storage_failure, "Could not prepare Song listing"});
    }

    std::vector<domain::Song> songs;
    int result = SQLITE_ROW;
    while ((result = sqlite3_step(statement.get())) == SQLITE_ROW) {
        const auto* raw_id = sqlite3_column_text(statement.get(), 0);
        if (raw_id == nullptr) {
            return domain::Result<std::vector<domain::Song>>::fail(
                {domain::ErrorCode::storage_failure, "Song listing contains a NULL id"});
        }
        const auto song = load_song(domain::SongId{reinterpret_cast<const char*>(raw_id)});
        if (!song) {
            return domain::Result<std::vector<domain::Song>>::fail(song.error());
        }
        songs.push_back(*song);
    }
    if (result != SQLITE_DONE) {
        return domain::Result<std::vector<domain::Song>>::fail(
            {domain::ErrorCode::storage_failure, "Could not iterate Song listing"});
    }
    return domain::Result<std::vector<domain::Song>>::ok(std::move(songs));
}

}  // namespace shekinah::storage
