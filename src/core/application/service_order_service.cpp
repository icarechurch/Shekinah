#include "shekinah/application/service_order_service.hpp"

#include <algorithm>
#include <string>

namespace shekinah::application {

ServiceOrderService::ServiceOrderService(domain::ServiceOrderId id) : id_(std::move(id)) {}

domain::Result<domain::ServiceOrderItem> ServiceOrderService::append(domain::ContentRef content) {
    if (content.id.index() == 0 && std::get<domain::SongId>(content.id).empty()) {
        return domain::Result<domain::ServiceOrderItem>::fail(
            {domain::ErrorCode::invalid_input, "A service-order content reference needs an id"});
    }
    if (content.id.index() == 1 && std::get<domain::SermonId>(content.id).empty()) {
        return domain::Result<domain::ServiceOrderItem>::fail(
            {domain::ErrorCode::invalid_input, "A service-order content reference needs an id"});
    }

    const auto item_id = domain::ServiceOrderItemId{"item-" + std::to_string(next_ordinal_)};
    domain::ServiceOrderItem item{item_id, std::move(content), next_ordinal_++};
    items_.push_back(item);
    return domain::Result<domain::ServiceOrderItem>::ok(std::move(item));
}

domain::Status ServiceOrderService::remove(const domain::ServiceOrderItemId& item_id) {
    const auto item = std::find_if(items_.begin(), items_.end(), [&](const auto& candidate) {
        return candidate.id == item_id;
    });
    if (item == items_.end()) {
        return domain::Status::fail({domain::ErrorCode::not_found, "Service-order item was not found"});
    }
    items_.erase(item);
    return domain::success();
}

domain::Status ServiceOrderService::move(
    const domain::ServiceOrderItemId& item_id, std::int64_t target_ordinal) {
    if (target_ordinal < 0) {
        return domain::Status::fail(
            {domain::ErrorCode::invalid_input, "Service-order position cannot be negative"});
    }
    const auto current = std::find_if(items_.begin(), items_.end(), [&](const auto& candidate) {
        return candidate.id == item_id;
    });
    if (current == items_.end()) {
        return domain::Status::fail({domain::ErrorCode::not_found, "Service-order item was not found"});
    }
    if (target_ordinal >= static_cast<std::int64_t>(items_.size())) {
        return domain::Status::fail(
            {domain::ErrorCode::invalid_input, "Service-order move position is outside the order"});
    }

    auto moved = *current;
    items_.erase(current);
    items_.insert(items_.begin() + static_cast<std::ptrdiff_t>(target_ordinal), std::move(moved));
    for (std::size_t index = 0; index != items_.size(); ++index) {
        items_[index].ordinal = static_cast<std::int64_t>(index);
    }
    next_ordinal_ = static_cast<std::int64_t>(items_.size());
    return domain::success();
}

}  // namespace shekinah::application
