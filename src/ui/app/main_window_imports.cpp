#include "shekinah/ui/main_window.hpp"

#include "shekinah/application/song_authoring.hpp"
#include "shekinah/application/song_text_importer.hpp"
#include "shekinah/storage/content_repository.hpp"

#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>

namespace shekinah::ui {

domain::Result<domain::Song> ShekinahWindow::import_song_file(
    const std::filesystem::path& source) {
    if (!repository()) {
        return domain::Result<domain::Song>::fail(
            {domain::ErrorCode::storage_failure, "Open a project before importing songs"});
    }
    std::ifstream input(source, std::ios::binary);
    if (!input) {
        return domain::Result<domain::Song>::fail(
            {domain::ErrorCode::io_failure, "Unable to open song text"});
    }
    const std::string text{std::istreambuf_iterator<char>{input}, std::istreambuf_iterator<char>{}};
    const auto parsed = application::SongTextImporter::parse(text);
    if (!parsed) {
        return domain::Result<domain::Song>::fail(parsed.error());
    }
    auto song = parsed.value();
    song.id = domain::SongId{allocate_song_id()};
    application::normalize_song_cue_ids(song);
    const auto saved = repository()->save_song(song);
    if (!saved) {
        return domain::Result<domain::Song>::fail(saved.error());
    }
    return saved;
}

}  // namespace shekinah::ui

