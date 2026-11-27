#include "song_import_coordinator.hpp"

#include "content_persistence_coordinator.hpp"
#include "shekinah/application/song_authoring.hpp"
#include "shekinah/application/song_text_importer.hpp"
#include "shekinah/storage/project_session.hpp"
#include "shekinah/storage/song_repository.hpp"

#include <fstream>
#include <iterator>
#include <string>
#include <utility>

namespace shekinah::ui {

SongImportCoordinator::SongImportCoordinator(
    storage::ProjectSession& session, ContentPersistenceCoordinator& persistence) noexcept
    : session_(session), persistence_(persistence) {}

domain::Result<domain::Song> SongImportCoordinator::import_file(
    domain::SongId id, const std::filesystem::path& source) const {
    if (!session_.songs()) {
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
    song.id = std::move(id);
    application::normalize_song_cue_ids(song);
    return persistence_.save_song(song);
}

}  // namespace shekinah::ui
