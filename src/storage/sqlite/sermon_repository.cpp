#include "shekinah/storage/content_repository.hpp"

#include "internal/sqlite_support.hpp"

#include <utility>
#include <vector>

namespace shekinah::storage {
namespace {

using sqlite_internal::Statement;
using sqlite_internal::bind_text;

}  // namespace

domain::Result<domain::SermonDeck> SqliteContentRepository::load_deck(const domain::SermonId& id) const {
    Statement deck_statement{store_.native_handle(),
                             "SELECT title FROM sermon_decks WHERE id=?1 AND archived=0 LIMIT 1;"};
    if (!deck_statement.ready()) {
        return domain::Result<domain::SermonDeck>::fail(
            {domain::ErrorCode::storage_failure, "Could not prepare Sermon load"});
    }
    if (const auto bound = bind_text(deck_statement, 1, id.value); !bound) {
        return domain::Result<domain::SermonDeck>::fail(bound.error());
    }
    if (sqlite3_step(deck_statement.get()) != SQLITE_ROW) {
        return domain::Result<domain::SermonDeck>::fail(
            {domain::ErrorCode::not_found, "Sermon deck was not found"});
    }
    const auto* raw_title = sqlite3_column_text(deck_statement.get(), 0);
    if (raw_title == nullptr) {
        return domain::Result<domain::SermonDeck>::fail(
            {domain::ErrorCode::storage_failure, "Sermon title is NULL"});
    }

    Statement slide_statement{store_.native_handle(),
                              "SELECT id,image_asset_id FROM sermon_slides "
                              "WHERE sermon_id=?1 ORDER BY ordinal ASC;"};
    if (!slide_statement.ready()) {
        return domain::Result<domain::SermonDeck>::fail(
            {domain::ErrorCode::storage_failure, "Could not prepare Sermon slide load"});
    }
    if (const auto bound = bind_text(slide_statement, 1, id.value); !bound) {
        return domain::Result<domain::SermonDeck>::fail(bound.error());
    }

    std::vector<domain::SermonSlide> slides;
    int result = SQLITE_ROW;
    while ((result = sqlite3_step(slide_statement.get())) == SQLITE_ROW) {
        const auto* raw_slide_id = sqlite3_column_text(slide_statement.get(), 0);
        const auto* raw_asset_id = sqlite3_column_text(slide_statement.get(), 1);
        if (raw_slide_id == nullptr || raw_asset_id == nullptr) {
            return domain::Result<domain::SermonDeck>::fail(
                {domain::ErrorCode::storage_failure, "Sermon slide contains NULL text"});
        }
        slides.push_back(domain::SermonSlide{
            domain::CueId{reinterpret_cast<const char*>(raw_slide_id)},
            domain::AssetId{reinterpret_cast<const char*>(raw_asset_id)},
        });
    }
    if (result != SQLITE_DONE) {
        return domain::Result<domain::SermonDeck>::fail(
            {domain::ErrorCode::storage_failure, "Could not iterate Sermon slides"});
    }
    return domain::Result<domain::SermonDeck>::ok(domain::SermonDeck{
        id,
        reinterpret_cast<const char*>(raw_title),
        std::move(slides),
    });
}

domain::Result<std::vector<domain::SermonDeck>> SqliteContentRepository::list_decks() const {
    Statement statement{store_.native_handle(),
                        "SELECT id FROM sermon_decks WHERE archived=0 "
                        "ORDER BY title COLLATE NOCASE ASC, id ASC;"};
    if (!statement.ready()) {
        return domain::Result<std::vector<domain::SermonDeck>>::fail(
            {domain::ErrorCode::storage_failure, "Could not prepare Sermon listing"});
    }

    std::vector<domain::SermonDeck> decks;
    int result = SQLITE_ROW;
    while ((result = sqlite3_step(statement.get())) == SQLITE_ROW) {
        const auto* raw_id = sqlite3_column_text(statement.get(), 0);
        if (raw_id == nullptr) {
            return domain::Result<std::vector<domain::SermonDeck>>::fail(
                {domain::ErrorCode::storage_failure, "Sermon listing contains a NULL id"});
        }
        const auto deck = load_deck(domain::SermonId{reinterpret_cast<const char*>(raw_id)});
        if (!deck) {
            return domain::Result<std::vector<domain::SermonDeck>>::fail(deck.error());
        }
        decks.push_back(*deck);
    }
    if (result != SQLITE_DONE) {
        return domain::Result<std::vector<domain::SermonDeck>>::fail(
            {domain::ErrorCode::storage_failure, "Could not iterate Sermon listing"});
    }
    return domain::Result<std::vector<domain::SermonDeck>>::ok(std::move(decks));
}

}  // namespace shekinah::storage
