#include "shekinah/storage/content_repository.hpp"

#include "internal/sqlite_support.hpp"

#include <string>

namespace shekinah::storage {
namespace {

using sqlite_internal::Statement;
using sqlite_internal::bind_text;
using sqlite_internal::sqlite_failure;
using sqlite_internal::step_done;

domain::Status archive_content(ProjectStore& store, std::string table, std::string kind, std::string id) {
    if (id.empty()) {
        return domain::Status::fail({domain::ErrorCode::invalid_input, "Reusable content needs an id"});
    }
    return store.transaction([&](sqlite3* database) {
        Statement references{database,
                             "SELECT COUNT(*) FROM service_order_items "
                             "WHERE content_kind=?1 AND content_id=?2;"};
        if (!references.ready()) {
            return sqlite_failure(database, "Could not prepare reference count");
        }
        if (const auto bound = bind_text(references, 1, kind); !bound) {
            return bound;
        }
        if (const auto bound = bind_text(references, 2, id); !bound) {
            return bound;
        }
        if (sqlite3_step(references.get()) != SQLITE_ROW) {
            return sqlite_failure(database, "Could not read reference count");
        }
        if (sqlite3_column_int64(references.get(), 0) != 0) {
            return domain::Status::fail(
                {domain::ErrorCode::invalid_input, "Remove service-order references before archiving content"});
        }

        const auto query = "UPDATE " + table + " SET archived=1 WHERE id=?1 AND archived=0;";
        Statement update{database, query.c_str()};
        if (!update.ready()) {
            return sqlite_failure(database, "Could not prepare archive operation");
        }
        if (const auto bound = bind_text(update, 1, id); !bound) {
            return bound;
        }
        if (const auto stepped = step_done(update, "Archive operation failed"); !stepped) {
            return stepped;
        }
        if (sqlite3_changes(database) == 0) {
            return domain::Status::fail({domain::ErrorCode::not_found, "Reusable content was not found"});
        }
        return domain::success();
    });
}

}  // namespace

domain::Status SqliteContentRepository::archive_song(const domain::SongId& id) {
    return archive_content(store_, "songs", "song", id.value);
}

domain::Status SqliteContentRepository::archive_deck(const domain::SermonId& id) {
    return archive_content(store_, "sermon_decks", "sermon", id.value);
}

}  // namespace shekinah::storage
