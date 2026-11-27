#include "shekinah/ui/main_window.hpp"

#include "library_catalog_coordinator.hpp"
#include "shekinah/ui/library_dock_widget.hpp"

namespace shekinah::ui {

void ShekinahWindow::refresh_library() {
    library_catalog_coordinator_->refresh(*library_dock_);
}

}  // namespace shekinah::ui
