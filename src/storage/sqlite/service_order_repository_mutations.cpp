#include "internal/sqlite_service_order_repository.hpp"

#include "internal/sqlite_support.hpp"

#include <string>
#include <utility>

namespace shekinah::storage {
namespace {

using sqlite_internal::Statement;
using sqlite_internal::bind_integer;
using sqlite_internal::bind_text;
using sqlite_internal::sqlite_failure;
using sqlite_internal::step_done;

}  // namespace

domain::Status SqliteServiceOrderRepository::ensure_service_order(
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

domain::Status SqliteServiceOrderRepository::remove_from_order(
    const domain::ServiceOrderItemId& item_id) {
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

}  // namespace shekinah::storage
