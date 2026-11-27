#include "shekinah/storage/content_repository.hpp"

#include "internal/sqlite_song_repository.hpp"
#include "internal/sqlite_sermon_repository.hpp"
#include "internal/sqlite_asset_repository.hpp"
#include "internal/sqlite_content_archive_repository.hpp"
#include "internal/sqlite_service_order_repository.hpp"

#include <memory>

namespace shekinah::storage {

SqliteContentRepository::SqliteContentRepository(ProjectStore& store)
    : store_(store),
      songs_(std::make_unique<SqliteSongRepository>(store)),
      sermons_(std::make_unique<SqliteSermonRepository>(store)),
      assets_(std::make_unique<SqliteAssetRepository>(store)),
      service_orders_(std::make_unique<SqliteServiceOrderRepository>(store)),
      archive_(std::make_unique<SqliteContentArchiveRepository>(store)) {}

SqliteContentRepository::~SqliteContentRepository() = default;

}  // namespace shekinah::storage
