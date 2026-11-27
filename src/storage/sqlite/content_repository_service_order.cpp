#include "shekinah/storage/content_repository.hpp"

#include "internal/sqlite_service_order_repository.hpp"

#include <utility>

namespace shekinah::storage {

domain::Status SqliteContentRepository::ensure_service_order(
    const domain::ServiceOrderId& id, std::string title) {
    return service_orders_->ensure_service_order(id, std::move(title));
}

domain::Result<domain::ServiceOrderItem> SqliteContentRepository::append_to_order(
    const domain::ServiceOrderId& order_id, domain::ContentRef content) {
    return service_orders_->append_to_order(order_id, std::move(content));
}

domain::Status SqliteContentRepository::remove_from_order(
    const domain::ServiceOrderItemId& item_id) {
    return service_orders_->remove_from_order(item_id);
}

domain::Status SqliteContentRepository::move_in_order(
    const domain::ServiceOrderItemId& item_id, std::int64_t target_ordinal) {
    return service_orders_->move_in_order(item_id, target_ordinal);
}

domain::Result<std::vector<domain::ServiceOrderItem>> SqliteContentRepository::load_order(
    const domain::ServiceOrderId& order_id) const {
    return service_orders_->load_order(order_id);
}

}  // namespace shekinah::storage
