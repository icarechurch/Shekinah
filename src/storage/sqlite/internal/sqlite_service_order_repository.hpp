#pragma once

#include "shekinah/storage/project_store.hpp"
#include "shekinah/storage/service_order_repository.hpp"

namespace shekinah::storage {

class SqliteServiceOrderRepository final : public ServiceOrderRepository {
public:
    explicit SqliteServiceOrderRepository(ProjectStore& store) : store_(store) {}

    domain::Status ensure_service_order(const domain::ServiceOrderId& id,
                                        std::string title) override;
    domain::Result<domain::ServiceOrderItem> append_to_order(
        const domain::ServiceOrderId& order_id, domain::ContentRef content) override;
    domain::Status remove_from_order(const domain::ServiceOrderItemId& item_id) override;
    domain::Status move_in_order(const domain::ServiceOrderItemId& item_id,
                                 std::int64_t target_ordinal) override;
    domain::Result<std::vector<domain::ServiceOrderItem>> load_order(
        const domain::ServiceOrderId& order_id) const override;

private:
    ProjectStore& store_;
};

}  // namespace shekinah::storage
