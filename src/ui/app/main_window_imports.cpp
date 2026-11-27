#include "shekinah/ui/main_window.hpp"

#include "song_import_coordinator.hpp"

namespace shekinah::ui {

domain::Result<domain::Song> ShekinahWindow::import_song_file(
    const std::filesystem::path& source) {
    return song_import_coordinator_->import_file(
        domain::SongId{allocate_song_id()}, source);
}

}  // namespace shekinah::ui

