#include "shekinah/ui/main_window.hpp"

#include "service_order_coordinator.hpp"

namespace shekinah::ui {

domain::Status ShekinahWindow::open_project(const std::filesystem::path& root) {
    const auto opened = session_.open(root);
    if (!opened) {
        return opened;
    }

    next_content_id_ = 1;

    const auto order = service_order_coordinator_->ensure_default_order();
    if (!order) {
        session_.close();
        return order;
    }
    refresh_library();
    refresh_service_order();
    return domain::success();
}

}  // namespace shekinah::ui
