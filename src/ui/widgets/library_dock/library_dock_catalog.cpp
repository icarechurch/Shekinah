#include "shekinah/ui/library_dock_widget.hpp"

#include <QListWidget>

namespace shekinah::ui {

void LibraryDockWidget::clear_contents() {
    assets_->clear();
    songs_->clear();
    sermons_->clear();
    update_tab_and_header_state();
    update_action_states();
    update_empty_state();
}

void LibraryDockWidget::purge_non_live_caches() { thumbnail_cache_.clear(); }

}  // namespace shekinah::ui
