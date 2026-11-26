#include "shekinah/storage/content_repository.hpp"

#include "internal/sqlite_support.hpp"

#include <algorithm>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace shekinah::storage {
namespace {

using sqlite_internal::Statement;
using sqlite_internal::bind_integer;
using sqlite_internal::bind_optional_text;
using sqlite_internal::bind_text;
using sqlite_internal::sqlite_failure;
using sqlite_internal::step_done;

domain::Result<bool> content_exists(sqlite3* database, const domain::ContentRef& content) {
    const char* query = content.kind == domain::ContentKind::song
                             ? "SELECT 1 FROM songs WHERE id=?1 AND archived=0 LIMIT 1;"
                             : "SELECT 1 FROM sermon_decks WHERE id=?1 AND archived=0 LIMIT 1;";
    Statement statement{database, query};
    if (!statement.ready()) {
        return domain::Result<bool>::fail(
            {domain::ErrorCode::storage_failure, "Could not prepare content lookup"});
    }
    const auto id = content.kind == domain::ContentKind::song
                        ? std::get<domain::SongId>(content.id).value
                        : std::get<domain::SermonId>(content.id).value;
    if (const auto bound = bind_text(statement, 1, id); !bound) {
        return domain::Result<bool>::fail(bound.error());
    }
    return domain::Result<bool>::ok(sqlite3_step(statement.get()) == SQLITE_ROW);
}

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

domain::Result<std::vector<domain::AssetRecord>> SqliteContentRepository::list_assets() const {
    Statement statement{store_.native_handle(),
                        "SELECT id,kind,sha256,relative_path,size_bytes FROM assets WHERE archived=0 "
                        "ORDER BY relative_path COLLATE NOCASE ASC, id ASC;"};
    if (!statement.ready()) {
        return domain::Result<std::vector<domain::AssetRecord>>::fail(
            {domain::ErrorCode::storage_failure, "Could not prepare Asset listing"});
    }

    std::vector<domain::AssetRecord> assets;
    int result = SQLITE_ROW;
    while ((result = sqlite3_step(statement.get())) == SQLITE_ROW) {
        const auto* raw_id = sqlite3_column_text(statement.get(), 0);
        const auto* raw_kind = sqlite3_column_text(statement.get(), 1);
        const auto* raw_hash = sqlite3_column_text(statement.get(), 2);
        const auto* raw_path = sqlite3_column_text(statement.get(), 3);
        if (raw_id == nullptr || raw_kind == nullptr || raw_hash == nullptr || raw_path == nullptr) {
            return domain::Result<std::vector<domain::AssetRecord>>::fail(
                {domain::ErrorCode::storage_failure, "Asset listing contains NULL metadata"});
        }
        assets.push_back(domain::AssetRecord{
            domain::AssetId{reinterpret_cast<const char*>(raw_id)},
            std::string_view{reinterpret_cast<const char*>(raw_kind)} == "video"
                ? domain::AssetKind::video
                : domain::AssetKind::image,
            reinterpret_cast<const char*>(raw_hash),
            reinterpret_cast<const char*>(raw_path),
            static_cast<std::uint64_t>(sqlite3_column_int64(statement.get(), 4)),
        });
    }
    if (result != SQLITE_DONE) {
        return domain::Result<std::vector<domain::AssetRecord>>::fail(
            {domain::ErrorCode::storage_failure, "Could not iterate Asset listing"});
    }
    return domain::Result<std::vector<domain::AssetRecord>>::ok(std::move(assets));
}

domain::Result<domain::AssetRecord> SqliteContentRepository::load_asset(
    const domain::AssetId& id) const {
    if (id.empty()) {
        return domain::Result<domain::AssetRecord>::fail(
            {domain::ErrorCode::invalid_input, "An asset id is required"});
    }
    Statement statement{store_.native_handle(),
                        "SELECT kind,sha256,relative_path,size_bytes FROM assets "
                        "WHERE id=?1 AND archived=0 LIMIT 1;"};
    if (!statement.ready()) {
        return domain::Result<domain::AssetRecord>::fail(
            {domain::ErrorCode::storage_failure, "Could not prepare Asset load"});
    }
    if (const auto bound = bind_text(statement, 1, id.value); !bound) {
        return domain::Result<domain::AssetRecord>::fail(bound.error());
    }
    if (sqlite3_step(statement.get()) != SQLITE_ROW) {
        return domain::Result<domain::AssetRecord>::fail(
            {domain::ErrorCode::not_found, "Asset was not found"});
    }
    const auto* raw_kind = sqlite3_column_text(statement.get(), 0);
    const auto* raw_hash = sqlite3_column_text(statement.get(), 1);
    const auto* raw_path = sqlite3_column_text(statement.get(), 2);
    if (raw_kind == nullptr || raw_hash == nullptr || raw_path == nullptr) {
        return domain::Result<domain::AssetRecord>::fail(
            {domain::ErrorCode::storage_failure, "Asset contains NULL metadata"});
    }
    return domain::Result<domain::AssetRecord>::ok(domain::AssetRecord{
        id,
        std::string_view{reinterpret_cast<const char*>(raw_kind)} == "video"
            ? domain::AssetKind::video
            : domain::AssetKind::image,
        reinterpret_cast<const char*>(raw_hash),
        reinterpret_cast<const char*>(raw_path),
        static_cast<std::uint64_t>(sqlite3_column_int64(statement.get(), 3)),
    });
}

domain::Status SqliteContentRepository::archive_song(const domain::SongId& id) {
    return archive_content(store_, "songs", "song", id.value);
}

domain::Status SqliteContentRepository::archive_deck(const domain::SermonId& id) {
    return archive_content(store_, "sermon_decks", "sermon", id.value);
}

domain::Status SqliteContentRepository::ensure_service_order(
    const domain::ServiceOrderId& id, std::string title) {
    if (id.empty() || title.empty()) {
        return domain::Status::fail(
            {domain::ErrorCode::invalid_input, "A service order needs a non-empty id and title"});
    }
    return store_.transaction([&](sqlite3* database) {
        Statement statement{database,
                            "INSERT INTO service_orders(id,title) VALUES(?1,?2) "
                            "ON CONFLICT(id) DO UPDATE SET title=excluded.title;"};
        if (!statement.ready()) {
            return sqlite_failure(database, "Could not prepare service-order save");
        }
        if (const auto bound = bind_text(statement, 1, id.value); !bound) {
            return bound;
        }
        if (const auto bound = bind_text(statement, 2, title); !bound) {
            return bound;
        }
        return step_done(statement, "Service-order save failed");
    });
}

domain::Result<domain::ServiceOrderItem> SqliteContentRepository::append_to_order(
    const domain::ServiceOrderId& order_id, domain::ContentRef content) {
    if (order_id.empty()) {
        return domain::Result<domain::ServiceOrderItem>::fail(
            {domain::ErrorCode::invalid_input, "A service-order id is required"});
    }

    std::optional<domain::ServiceOrderItem> item;
    const auto status = store_.transaction([&](sqlite3* database) {
        Statement order_lookup{database, "SELECT 1 FROM service_orders WHERE id=?1 LIMIT 1;"};
        if (!order_lookup.ready()) {
            return sqlite_failure(database, "Could not prepare service-order lookup");
        }
        if (const auto bound = bind_text(order_lookup, 1, order_id.value); !bound) {
            return bound;
        }
        if (sqlite3_step(order_lookup.get()) != SQLITE_ROW) {
            return domain::Status::fail({domain::ErrorCode::not_found, "Service order was not found"});
        }

        const auto exists = content_exists(database, content);
        if (!exists) {
            return domain::Status::fail(exists.error());
        }
        if (!*exists) {
            return domain::Status::fail({domain::ErrorCode::not_found, "Reusable content was not found"});
        }

        Statement ordinal_lookup{database,
                                 "SELECT COALESCE(MAX(ordinal), -1) + 1 FROM service_order_items "
                                 "WHERE service_order_id=?1;"};
        if (!ordinal_lookup.ready()) {
            return sqlite_failure(database, "Could not prepare service-order ordinal lookup");
        }
        if (const auto bound = bind_text(ordinal_lookup, 1, order_id.value); !bound) {
            return bound;
        }
        if (sqlite3_step(ordinal_lookup.get()) != SQLITE_ROW) {
            return sqlite_failure(database, "Could not read service-order ordinal");
        }
        const auto ordinal = sqlite3_column_int64(ordinal_lookup.get(), 0);
        const auto item_id = domain::ServiceOrderItemId{
            order_id.value + "-item-" + std::to_string(ordinal)};
        const auto kind = content.kind == domain::ContentKind::song ? "song" : "sermon";
        const auto content_id = content.kind == domain::ContentKind::song
                                    ? std::get<domain::SongId>(content.id).value
                                    : std::get<domain::SermonId>(content.id).value;

        Statement insert{database,
                         "INSERT INTO service_order_items(id,service_order_id,content_kind,content_id,ordinal) "
                         "VALUES(?1,?2,?3,?4,?5);"};
        if (!insert.ready()) {
            return sqlite_failure(database, "Could not prepare service-order item save");
        }
        if (const auto bound = bind_text(insert, 1, item_id.value); !bound) {
            return bound;
        }
        if (const auto bound = bind_text(insert, 2, order_id.value); !bound) {
            return bound;
        }
        if (const auto bound = bind_text(insert, 3, kind); !bound) {
            return bound;
        }
        if (const auto bound = bind_text(insert, 4, content_id); !bound) {
            return bound;
        }
        if (const auto bound = bind_integer(insert, 5, ordinal); !bound) {
            return bound;
        }
        if (const auto stepped = step_done(insert, "Service-order item save failed"); !stepped) {
            return stepped;
        }
        item = domain::ServiceOrderItem{item_id, std::move(content), ordinal};
        return domain::success();
    });

    if (!status) {
        return domain::Result<domain::ServiceOrderItem>::fail(status.error());
    }
    return domain::Result<domain::ServiceOrderItem>::ok(std::move(*item));
}

domain::Status SqliteContentRepository::remove_from_order(const domain::ServiceOrderItemId& item_id) {
    return store_.transaction([&](sqlite3* database) {
        Statement statement{database, "DELETE FROM service_order_items WHERE id=?1;"};
        if (!statement.ready()) {
            return sqlite_failure(database, "Could not prepare service-order item delete");
        }
        if (const auto bound = bind_text(statement, 1, item_id.value); !bound) {
            return bound;
        }
        if (const auto stepped = step_done(statement, "Service-order item delete failed"); !stepped) {
            return stepped;
        }
        if (sqlite3_changes(database) == 0) {
            return domain::Status::fail({domain::ErrorCode::not_found, "Service-order item was not found"});
        }
        return domain::success();
    });
}

domain::Status SqliteContentRepository::move_in_order(
    const domain::ServiceOrderItemId& item_id, std::int64_t target_ordinal) {
    if (item_id.empty() || target_ordinal < 0) {
        return domain::Status::fail(
            {domain::ErrorCode::invalid_input, "A service-order move needs an item and non-negative position"});
    }

    return store_.transaction([&](sqlite3* database) {
        Statement owner_lookup{database,
                               "SELECT service_order_id FROM service_order_items WHERE id=?1 LIMIT 1;"};
        if (!owner_lookup.ready()) {
            return sqlite_failure(database, "Could not prepare service-order move lookup");
        }
        if (const auto bound = bind_text(owner_lookup, 1, item_id.value); !bound) {
            return bound;
        }
        if (sqlite3_step(owner_lookup.get()) != SQLITE_ROW) {
            return domain::Status::fail(
                {domain::ErrorCode::not_found, "Service-order item was not found"});
        }
        const auto* raw_order_id = sqlite3_column_text(owner_lookup.get(), 0);
        if (raw_order_id == nullptr) {
            return domain::Status::fail(
                {domain::ErrorCode::storage_failure, "Service-order item has no owning order"});
        }
        const std::string order_id{reinterpret_cast<const char*>(raw_order_id)};

        Statement list{database,
                       "SELECT id FROM service_order_items WHERE service_order_id=?1 "
                       "ORDER BY ordinal ASC, id ASC;"};
        if (!list.ready()) {
            return sqlite_failure(database, "Could not prepare service-order move list");
        }
        if (const auto bound = bind_text(list, 1, order_id); !bound) {
            return bound;
        }
        std::vector<std::string> item_ids;
        while (sqlite3_step(list.get()) == SQLITE_ROW) {
            const auto* raw_id = sqlite3_column_text(list.get(), 0);
            if (raw_id == nullptr) {
                return domain::Status::fail(
                    {domain::ErrorCode::storage_failure, "Service-order item has no id"});
            }
            item_ids.emplace_back(reinterpret_cast<const char*>(raw_id));
        }

        const auto current = std::find(item_ids.begin(), item_ids.end(), item_id.value);
        if (current == item_ids.end()) {
            return domain::Status::fail(
                {domain::ErrorCode::not_found, "Service-order item was not found"});
        }
        if (target_ordinal >= static_cast<std::int64_t>(item_ids.size())) {
            return domain::Status::fail(
                {domain::ErrorCode::invalid_input, "Service-order move position is outside the order"});
        }
        const auto moved_item = *current;
        item_ids.erase(current);
        item_ids.insert(item_ids.begin() + static_cast<std::ptrdiff_t>(target_ordinal), moved_item);

        Statement update{database, "UPDATE service_order_items SET ordinal=?1 WHERE id=?2;"};
        if (!update.ready()) {
            return sqlite_failure(database, "Could not prepare service-order move update");
        }
        for (std::size_t index = 0; index < item_ids.size(); ++index) {
            if (const auto reset = sqlite3_reset(update.get()); reset != SQLITE_OK) {
                return sqlite_failure(database, "Could not reset service-order move update");
            }
            if (const auto clear = sqlite3_clear_bindings(update.get()); clear != SQLITE_OK) {
                return sqlite_failure(database, "Could not clear service-order move update");
            }
            if (const auto bound = bind_integer(update, 1, static_cast<std::int64_t>(index)); !bound) {
                return bound;
            }
            if (const auto bound = bind_text(update, 2, item_ids[index]); !bound) {
                return bound;
            }
            if (const auto stepped = step_done(update, "Service-order move update failed"); !stepped) {
                return stepped;
            }
        }
        return domain::success();
    });
}

domain::Result<std::vector<domain::ServiceOrderItem>> SqliteContentRepository::load_order(
    const domain::ServiceOrderId& order_id) const {
    Statement statement{store_.native_handle(),
                        "SELECT id,content_kind,content_id,ordinal FROM service_order_items "
                        "WHERE service_order_id=?1 ORDER BY ordinal ASC, id ASC;"};
    if (!statement.ready()) {
        return domain::Result<std::vector<domain::ServiceOrderItem>>::fail(
            {domain::ErrorCode::storage_failure, "Could not prepare service-order load"});
    }
    if (const auto bound = bind_text(statement, 1, order_id.value); !bound) {
        return domain::Result<std::vector<domain::ServiceOrderItem>>::fail(bound.error());
    }

    std::vector<domain::ServiceOrderItem> items;
    while (sqlite3_step(statement.get()) == SQLITE_ROW) {
        const auto* raw_item_id = sqlite3_column_text(statement.get(), 0);
        const auto* raw_kind = sqlite3_column_text(statement.get(), 1);
        const auto* raw_content_id = sqlite3_column_text(statement.get(), 2);
        if (raw_item_id == nullptr || raw_kind == nullptr || raw_content_id == nullptr) {
            return domain::Result<std::vector<domain::ServiceOrderItem>>::fail(
                {domain::ErrorCode::storage_failure, "Service-order row contains NULL text"});
        }
        const std::string kind{reinterpret_cast<const char*>(raw_kind)};
        const std::string content_id{reinterpret_cast<const char*>(raw_content_id)};
        domain::ContentRef content = kind == "song"
                                          ? domain::ContentRef::song(content_id)
                                          : domain::ContentRef::sermon(content_id);
        if (kind != "song" && kind != "sermon") {
            return domain::Result<std::vector<domain::ServiceOrderItem>>::fail(
                {domain::ErrorCode::storage_failure, "Service-order row has an unknown content kind"});
        }
        items.push_back(domain::ServiceOrderItem{
            domain::ServiceOrderItemId{reinterpret_cast<const char*>(raw_item_id)},
            std::move(content),
            sqlite3_column_int64(statement.get(), 3),
        });
    }
    return domain::Result<std::vector<domain::ServiceOrderItem>>::ok(std::move(items));
}

domain::Result<std::size_t> SqliteContentRepository::song_count() const {
    Statement statement{store_.native_handle(), "SELECT COUNT(*) FROM songs WHERE archived=0;"};
    if (!statement.ready()) {
        return domain::Result<std::size_t>::fail(
            {domain::ErrorCode::storage_failure, "Could not prepare Song count"});
    }
    if (sqlite3_step(statement.get()) != SQLITE_ROW) {
        return domain::Result<std::size_t>::fail(
            {domain::ErrorCode::storage_failure, "Could not read Song count"});
    }
    return domain::Result<std::size_t>::ok(static_cast<std::size_t>(sqlite3_column_int64(statement.get(), 0)));
}

}  // namespace shekinah::storage
