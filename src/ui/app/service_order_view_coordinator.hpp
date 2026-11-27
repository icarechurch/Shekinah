#pragma once

#include "shekinah/domain/model.hpp"
#include "shekinah/ui/service_order_view.hpp"

#include <vector>

namespace shekinah::application {
class PresentationService;
}

namespace shekinah::storage {
class ProjectSession;
}

namespace shekinah::ui {

class ServiceOrderViewCoordinator final {
public:
    ServiceOrderViewCoordinator(storage::ProjectSession& session,
                                application::PresentationService& presentation) noexcept;

    [[nodiscard]] std::vector<ServiceOrderView> make_views(
        const std::vector<domain::ServiceOrderItem>& items,
        const domain::ProgramState& program);

private:
    storage::ProjectSession& session_;
    application::PresentationService& presentation_;
};

}  // namespace shekinah::ui
