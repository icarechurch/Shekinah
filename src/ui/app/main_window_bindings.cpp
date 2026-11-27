#include "shekinah/ui/main_window.hpp"

namespace shekinah::ui {

void ShekinahWindow::configure_feature_handlers() {
    configure_library_handlers();
    configure_operator_feature_handlers();
}

}  // namespace shekinah::ui
