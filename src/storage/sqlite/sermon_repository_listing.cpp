#include "shekinah/storage/content_repository.hpp"

#include "internal/sqlite_support.hpp"

#include <utility>
#include <vector>

namespace shekinah::storage {
namespace {

using sqlite_internal::Statement;

}  // namespace

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
