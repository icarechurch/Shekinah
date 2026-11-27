#include "internal/sqlite_sermon_repository.hpp"

#include "internal/sqlite_support.hpp"

#include <utility>

namespace shekinah::storage {
namespace {

using sqlite_internal::Statement;
using sqlite_internal::bind_integer;
using sqlite_internal::bind_text;
using sqlite_internal::sqlite_failure;
using sqlite_internal::step_done;

}  // namespace

domain::Result<domain::SermonDeck> SqliteSermonRepository::save_deck(
    const domain::SermonDeck& deck) {
    if (deck.id.empty() || deck.title.empty()) {
        return domain::Result<domain::SermonDeck>::fail(
            {domain::ErrorCode::invalid_input, "A Sermon deck needs a non-empty id and title"});
    }

    const auto status = store_.transaction([&](sqlite3* database) {
        Statement deck_insert{database,
                              "INSERT INTO sermon_decks(id,title) VALUES(?1,?2) "
                              "ON CONFLICT(id) DO UPDATE SET title=excluded.title, archived=0;"};
        if (!deck_insert.ready()) {
            return sqlite_failure(database, "Could not prepare Sermon save");
        }
        if (const auto bound = bind_text(deck_insert, 1, deck.id.value); !bound) {
            return bound;
        }
        if (const auto bound = bind_text(deck_insert, 2, deck.title); !bound) {
            return bound;
        }
        if (const auto stepped = step_done(deck_insert, "Sermon save failed"); !stepped) {
            return stepped;
        }

        Statement delete_slides{database, "DELETE FROM sermon_slides WHERE sermon_id=?1;"};
        if (!delete_slides.ready()) {
            return sqlite_failure(database, "Could not prepare Sermon cleanup");
        }
        if (const auto bound = bind_text(delete_slides, 1, deck.id.value); !bound) {
            return bound;
        }
        if (const auto stepped = step_done(delete_slides, "Sermon cleanup failed"); !stepped) {
            return stepped;
        }

        Statement slide_insert{database,
                               "INSERT INTO sermon_slides(id,sermon_id,ordinal,image_asset_id) "
                               "VALUES(?1,?2,?3,?4);"};
        if (!slide_insert.ready()) {
            return sqlite_failure(database, "Could not prepare Sermon slide save");
        }
        for (std::size_t index = 0; index != deck.slides.size(); ++index) {
            const auto& slide = deck.slides[index];
            sqlite3_reset(slide_insert.get());
            sqlite3_clear_bindings(slide_insert.get());
            if (const auto bound = bind_text(slide_insert, 1, slide.id.value); !bound) {
                return bound;
            }
            if (const auto bound = bind_text(slide_insert, 2, deck.id.value); !bound) {
                return bound;
            }
            if (const auto bound =
                    bind_integer(slide_insert, 3, static_cast<std::int64_t>(index));
                !bound) {
                return bound;
            }
            if (const auto bound = bind_text(slide_insert, 4, slide.image.value); !bound) {
                return bound;
            }
            if (const auto stepped = step_done(slide_insert, "Sermon slide save failed"); !stepped) {
                return stepped;
            }
        }
        return domain::success();
    });

    if (!status) {
        return domain::Result<domain::SermonDeck>::fail(status.error());
    }
    return domain::Result<domain::SermonDeck>::ok(deck);
}

}  // namespace shekinah::storage
