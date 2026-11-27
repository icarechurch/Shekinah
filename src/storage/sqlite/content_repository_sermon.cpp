#include "shekinah/storage/content_repository.hpp"

#include "internal/sqlite_sermon_repository.hpp"

namespace shekinah::storage {

domain::Result<domain::SermonDeck> SqliteContentRepository::save_deck(
    const domain::SermonDeck& deck) {
    return sermons_->save_deck(deck);
}

domain::Result<domain::SermonDeck> SqliteContentRepository::load_deck(
    const domain::SermonId& id) const {
    return sermons_->load_deck(id);
}

domain::Result<std::vector<domain::SermonDeck>> SqliteContentRepository::list_decks() const {
    return sermons_->list_decks();
}

}  // namespace shekinah::storage
