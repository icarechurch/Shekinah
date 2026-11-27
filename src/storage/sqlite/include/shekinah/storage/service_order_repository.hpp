#pragma once

#include "shekinah/domain/model.hpp"
#include "shekinah/domain/result.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace shekinah::storage {

class ServiceOrderRepository {
public:
    virtual ~ServiceOrderRepository() = default;

    virtual domain::Status ensure_service_order(const domain::ServiceOrderId& id,
                                                std::string title) = 0;
    virtual domain::Result<domain::ServiceOrderItem> append_to_order(
        const domain::ServiceOrderId& order_id, domain::ContentRef content) = 0;
    virtual domain::Status remove_from_order(const domain::ServiceOrderItemId& item_id) = 0;
    virtual domain::Status move_in_order(const domain::ServiceOrderItemId& item_id,
                                         std::int64_t target_ordinal) = 0;
    virtual domain::Result<std::vector<domain::ServiceOrderItem>> load_order(
        const domain::ServiceOrderId& order_id) const = 0;
};

}  // namespace shekinah::storage
