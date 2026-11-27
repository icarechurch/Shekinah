#include "shekinah/storage/content_repository.hpp"

#include "internal/sqlite_song_repository.hpp"
#include "internal/sqlite_sermon_repository.hpp"

#include <memory>

namespace shekinah::storage {

SqliteContentRepository::SqliteContentRepository(ProjectStore& store)
    : store_(store),
      songs_(std::make_unique<SqliteSongRepository>(store)),
      sermons_(std::make_unique<SqliteSermonRepository>(store)) {}

SqliteContentRepository::~SqliteContentRepository() = default;

}  // namespace shekinah::storage
