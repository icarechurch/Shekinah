#include "internal/sqlite_service_order_repository.hpp"

#include "internal/sqlite_support.hpp"

#include <string>
#include <utility>
#include <vector>

namespace shekinah::storage {

domain::Result<std::vector<domain::ServiceOrderItem>> SqliteServiceOrderRepository::load_order(
    const domain::ServiceOrderId& order_id) const {
    using sqlite_internal::Statement;
    using sqlite_internal::bind_text;

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

}  // namespace shekinah::storage
