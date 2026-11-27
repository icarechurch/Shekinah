#include "shekinah/ui/main_window.hpp"

#include "shekinah/storage/content_repository.hpp"

namespace shekinah::ui {

domain::Status ShekinahWindow::open_project(const std::filesystem::path& root) {
    const auto opened = session_.open(root);
    if (!opened) {
        return opened;
    }

    next_content_id_ = 1;

    const auto order = session_.repository()->ensure_service_order(
        domain::ServiceOrderId{"default-service"}, "Service Order");
    if (!order) {
        session_.close();
        return order;
    }
    refresh_library();
    refresh_service_order();
    return domain::success();
}

}  // namespace shekinah::ui
