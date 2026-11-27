#include "shekinah/ui/main_window.hpp"

#include "content_id_allocator.hpp"
#include "song_import_coordinator.hpp"

namespace shekinah::ui {

domain::Result<domain::Song> ShekinahWindow::import_song_file(
    const std::filesystem::path& source) {
    return song_import_coordinator_->import_file(
        content_id_allocator_->allocate_song_id(), source);
}

}  // namespace shekinah::ui

