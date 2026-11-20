#pragma once

#include "shekinah/domain/model.hpp"
#include "shekinah/domain/result.hpp"
#include "shekinah/storage/project_store.hpp"

#include <cstddef>
#include <vector>

namespace shekinah::storage {

class SqliteContentRepository {
public:
    explicit SqliteContentRepository(ProjectStore& store) : store_(store) {}

    domain::Result<domain::Song> save_song(const domain::Song& song);
    domain::Result<domain::Song> load_song(const domain::SongId& id) const;
    domain::Result<domain::SermonDeck> save_deck(const domain::SermonDeck& deck);
    domain::Result<domain::SermonDeck> load_deck(const domain::SermonId& id) const;
    domain::Status archive_song(const domain::SongId& id);
    domain::Status archive_deck(const domain::SermonId& id);
    domain::Status ensure_service_order(const domain::ServiceOrderId& id, std::string title);
    domain::Result<domain::ServiceOrderItem> append_to_order(
        const domain::ServiceOrderId& order_id, domain::ContentRef content);
    domain::Status remove_from_order(const domain::ServiceOrderItemId& item_id);
    domain::Result<std::vector<domain::ServiceOrderItem>> load_order(
        const domain::ServiceOrderId& order_id) const;
    domain::Result<std::size_t> song_count() const;

private:
    ProjectStore& store_;
};

}  // namespace shekinah::storage
