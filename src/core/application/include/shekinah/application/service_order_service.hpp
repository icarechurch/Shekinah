#pragma once

#include "shekinah/domain/model.hpp"
#include "shekinah/domain/result.hpp"

#include <vector>

namespace shekinah::application {

class ServiceOrderService {
public:
    explicit ServiceOrderService(domain::ServiceOrderId id);

    [[nodiscard]] const domain::ServiceOrderId& id() const noexcept { return id_; }
    [[nodiscard]] const std::vector<domain::ServiceOrderItem>& items() const noexcept { return items_; }

    domain::Result<domain::ServiceOrderItem> append(domain::ContentRef content);
    domain::Status remove(const domain::ServiceOrderItemId& item_id);

private:
    domain::ServiceOrderId id_;
    std::vector<domain::ServiceOrderItem> items_;
    std::int64_t next_ordinal_{0};
};

}  // namespace shekinah::application
