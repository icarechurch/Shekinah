#include "internal/sqlite_service_order_repository.hpp"

#include "internal/sqlite_support.hpp"

#include <algorithm>
#include <cstddef>
#include <string>
#include <vector>

namespace shekinah::storage {

domain::Status SqliteServiceOrderRepository::move_in_order(
    const domain::ServiceOrderItemId& item_id, std::int64_t target_ordinal) {
    using sqlite_internal::Statement;
    using sqlite_internal::bind_integer;
    using sqlite_internal::bind_text;
    using sqlite_internal::sqlite_failure;
    using sqlite_internal::step_done;

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

}  // namespace shekinah::storage
