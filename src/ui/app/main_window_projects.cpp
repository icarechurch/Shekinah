#include "shekinah/ui/main_window.hpp"

#include "project_lifecycle_coordinator.hpp"

namespace shekinah::ui {

domain::Status ShekinahWindow::open_project(const std::filesystem::path& root) {
    const auto opened = project_lifecycle_coordinator_->open(root);
    if (!opened) {
        return opened;
    }

    next_content_id_ = 1;
    refresh_library();
    refresh_service_order();
    return domain::success();
}

void ShekinahWindow::close_project() {
    project_lifecycle_coordinator_->close();
    refresh_library();
    refresh_service_order();
}

}  // namespace shekinah::ui
