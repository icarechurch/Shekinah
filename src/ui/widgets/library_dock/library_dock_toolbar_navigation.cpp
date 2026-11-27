#include "shekinah/ui/library_dock_widget.hpp"

namespace shekinah::ui {

void LibraryDockWidget::setup_toolbar_navigation(QWidget* toolbar, FlowLayout* toolbar_layout) {
    setup_tab_navigation(toolbar, toolbar_layout);
    setup_search_setup(toolbar, toolbar_layout);
    setup_asset_filter(toolbar, toolbar_layout);
}

}  // namespace shekinah::ui
