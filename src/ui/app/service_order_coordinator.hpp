#pragma once

#include "shekinah/domain/model.hpp"
#include "shekinah/domain/result.hpp"

#include <cstdint>
#include <vector>

namespace shekinah::storage {
class ProjectSession;
}

namespace shekinah::ui {

class ServiceOrderCoordinator final {
public:
    explicit ServiceOrderCoordinator(storage::ProjectSession& session) noexcept;

    domain::Status ensure_default_order();
    domain::Result<domain::ServiceOrderItem> append(domain::ContentRef content);
    domain::Status remove(const domain::ServiceOrderItemId& item_id);
    domain::Status move(const domain::ServiceOrderItemId& item_id, std::int64_t target_ordinal);
    domain::Result<std::vector<domain::ServiceOrderItem>> load_default_order() const;

private:
    storage::ProjectSession& session_;
};

}  // namespace shekinah::ui
