#include "shekinah/ui/main_window.hpp"

#include "content_id_allocator.hpp"
#include "project_lifecycle_coordinator.hpp"

namespace shekinah::ui {

domain::Status ShekinahWindow::open_project(const std::filesystem::path& root) {
    const auto opened = project_lifecycle_coordinator_->open(root);
    if (!opened) {
        return opened;
    }

    content_id_allocator_->reset();
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
