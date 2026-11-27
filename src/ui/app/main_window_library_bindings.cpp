#include "shekinah/ui/main_window.hpp"

#include "service_order_coordinator.hpp"
#include "shekinah/ui/library_dock_widget.hpp"

#include <utility>

namespace shekinah::ui {

void ShekinahWindow::configure_library_handlers() {
    library_dock_->set_import_asset_handler([this] { import_asset(); });
    library_dock_->set_import_song_handler([this] { import_song_text(); });
    library_dock_->set_new_song_handler([this] { create_song(); });
    library_dock_->set_import_sermon_handler([this] { import_sermon_deck(); });
    library_dock_->set_new_sermon_handler([this] { create_sermon(); });
    library_dock_->set_edit_sermon_handler([this](domain::SermonId id) { edit_sermon(id); });
    library_dock_->set_edit_song_handler([this](domain::SongId id) { edit_song(id); });

    library_dock_->set_add_content_handler([this](domain::ContentRef content) {
        const auto appended = service_order_coordinator_->append(std::move(content));
        if (appended) {
            refresh_service_order();
        }
    });
}

}  // namespace shekinah::ui
