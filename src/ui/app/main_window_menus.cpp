#include "shekinah/ui/main_window.hpp"

#include <QMainWindow>

namespace shekinah::ui {

void ShekinahWindow::configure_window_actions() {
    configure_window_menu();
    configure_file_menu();
    configure_presentation_menu();
    configure_view_menu();
}

}  // namespace shekinah::ui
