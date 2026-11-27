#include "project_lifecycle_coordinator.hpp"

#include "service_order_coordinator.hpp"
#include "shekinah/application/presentation_service.hpp"
#include "shekinah/storage/project_session.hpp"

namespace shekinah::ui {

ProjectLifecycleCoordinator::ProjectLifecycleCoordinator(
    storage::ProjectSession& session,
    application::PresentationService& presentation,
    ServiceOrderCoordinator& service_orders) noexcept
    : session_(session), presentation_(presentation), service_orders_(service_orders) {}

domain::Status ProjectLifecycleCoordinator::open(const std::filesystem::path& root) {
    const auto opened = session_.open(root);
    if (!opened) {
        return opened;
    }

    const auto order = service_orders_.ensure_default_order();
    if (!order) {
        session_.close();
        return order;
    }
    return domain::success();
}

void ProjectLifecycleCoordinator::close() noexcept {
    session_.close();
    presentation_.reset();
}

}  // namespace shekinah::ui
