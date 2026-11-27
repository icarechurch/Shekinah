#include "shekinah/storage/content_repository.hpp"

#include "internal/song_repository_writes.hpp"
#include "internal/sqlite_support.hpp"

#include <optional>
#include <string>
#include <utility>

namespace shekinah::storage {
namespace {

using sqlite_internal::Statement;
using sqlite_internal::bind_optional_text;
using sqlite_internal::bind_text;
using sqlite_internal::sqlite_failure;
using sqlite_internal::step_done;

}  // namespace

domain::Result<domain::Song> SqliteContentRepository::save_song(const domain::Song& song) {
    if (song.id.empty() || song.title.empty()) {
        return domain::Result<domain::Song>::fail(
            {domain::ErrorCode::invalid_input, "A Song needs a non-empty id and title"});
    }

    const auto status = store_.transaction([&](sqlite3* database) {
        Statement song_statement{
            database,
            "INSERT INTO songs(id,title,default_background_asset_id) VALUES(?1,?2,?3) "
            "ON CONFLICT(id) DO UPDATE SET title=excluded.title, "
            "default_background_asset_id=excluded.default_background_asset_id, archived=0;"};
        if (!song_statement.ready()) {
            return sqlite_failure(database, "Could not prepare Song save");
        }
        if (const auto bound = bind_text(song_statement, 1, song.id.value); !bound) {
            return bound;
        }
        if (const auto bound = bind_text(song_statement, 2, song.title); !bound) {
            return bound;
        }
        const std::optional<std::string> background = song.default_background.has_value()
                                                          ? std::optional<std::string>{
                                                                song.default_background->asset.value}
                                                          : std::nullopt;
        if (const auto bound = bind_optional_text(song_statement, 3, background); !bound) {
            return bound;
        }
        if (const auto stepped = step_done(song_statement, "Song save failed"); !stepped) {
            return stepped;
        }

        for (const char* query : {"DELETE FROM song_slides WHERE song_id=?1;",
                                  "DELETE FROM song_slide_breaks WHERE song_id=?1;",
                                  "DELETE FROM song_reflow_documents WHERE song_id=?1;"}) {
            Statement deletion{database, query};
            if (!deletion.ready()) {
                return sqlite_failure(database, "Could not prepare Song cleanup");
            }
            if (const auto bound = bind_text(deletion, 1, song.id.value); !bound) {
                return bound;
            }
            if (const auto stepped = step_done(deletion, "Song cleanup failed"); !stepped) {
                return stepped;
            }
        }

        if (const auto reflow = song_repository_internal::write_reflow(database, song); !reflow) {
            return reflow;
        }
        if (const auto slides = song_repository_internal::write_slides(database, song); !slides) {
            return slides;
        }
        return domain::success();
    });

    if (!status) {
        return domain::Result<domain::Song>::fail(status.error());
    }
    return domain::Result<domain::Song>::ok(song);
}

}  // namespace shekinah::storage
