#include "service_order_coordinator.hpp"

#include "shekinah/storage/project_session.hpp"
#include "shekinah/storage/service_order_repository.hpp"

#include <utility>

namespace shekinah::ui {
namespace {

const domain::ServiceOrderId kDefaultOrderId{"default-service"};

storage::ServiceOrderRepository* repository(storage::ProjectSession& session) noexcept {
    return session.service_orders();
}

}  // namespace

ServiceOrderCoordinator::ServiceOrderCoordinator(storage::ProjectSession& session) noexcept
    : session_(session) {}

domain::Status ServiceOrderCoordinator::ensure_default_order() {
    auto* orders = repository(session_);
    if (orders == nullptr) {
        return domain::Status::fail(
            {domain::ErrorCode::storage_failure, "Open a project before configuring service order"});
    }
    return orders->ensure_service_order(kDefaultOrderId, "Service Order");
}

domain::Result<domain::ServiceOrderItem> ServiceOrderCoordinator::append(
    domain::ContentRef content) {
    auto* orders = repository(session_);
    if (orders == nullptr) {
        return domain::Result<domain::ServiceOrderItem>::fail(
            {domain::ErrorCode::storage_failure, "Open a project before editing service order"});
    }
    return orders->append_to_order(kDefaultOrderId, std::move(content));
}

domain::Status ServiceOrderCoordinator::remove(const domain::ServiceOrderItemId& item_id) {
    auto* orders = repository(session_);
    if (orders == nullptr) {
        return domain::Status::fail(
            {domain::ErrorCode::storage_failure, "Open a project before editing service order"});
    }
    return orders->remove_from_order(item_id);
}

domain::Status ServiceOrderCoordinator::move(
    const domain::ServiceOrderItemId& item_id, std::int64_t target_ordinal) {
    auto* orders = repository(session_);
    if (orders == nullptr) {
        return domain::Status::fail(
            {domain::ErrorCode::storage_failure, "Open a project before editing service order"});
    }
    return orders->move_in_order(item_id, target_ordinal);
}

domain::Result<std::vector<domain::ServiceOrderItem>>
ServiceOrderCoordinator::load_default_order() const {
    const auto* orders = repository(session_);
    if (orders == nullptr) {
        return domain::Result<std::vector<domain::ServiceOrderItem>>::fail(
            {domain::ErrorCode::storage_failure, "Open a project before loading service order"});
    }
    return orders->load_order(kDefaultOrderId);
}

}  // namespace shekinah::ui
