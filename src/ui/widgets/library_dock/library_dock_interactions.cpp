#include "shekinah/ui/library_dock_widget.hpp"

namespace shekinah::ui {

void LibraryDockWidget::setup_interactions() {
    setup_song_interactions();
    setup_sermon_interactions();
    setup_search_interactions();
}

}  // namespace shekinah::ui
