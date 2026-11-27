#include "shekinah/storage/content_repository.hpp"

#include "internal/sqlite_support.hpp"

#include <optional>
#include <string>
#include <utility>

namespace shekinah::storage {
namespace {

using sqlite_internal::Statement;
using sqlite_internal::bind_integer;
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

}  // namespace

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

}  // namespace shekinah::storage
