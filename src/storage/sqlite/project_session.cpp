#include "shekinah/storage/project_session.hpp"

#include "internal/sqlite_asset_repository.hpp"
#include "internal/sqlite_content_archive_repository.hpp"
#include "internal/sqlite_sermon_repository.hpp"
#include "internal/sqlite_service_order_repository.hpp"
#include "internal/sqlite_song_repository.hpp"

#include "shekinah/storage/project_store.hpp"
#include "shekinah/storage/asset_importer.hpp"
#include "shekinah/storage/sermon_deck_importer.hpp"

#include <utility>

namespace shekinah::storage {

ProjectSession::ProjectSession() = default;
ProjectSession::~ProjectSession() = default;

domain::Status ProjectSession::open(const std::filesystem::path& root) {
    close();

    auto opened = ProjectStore::create_or_open(root);
    if (!opened) {
        return domain::Status::fail(opened.error());
    }

    store_ = std::make_unique<ProjectStore>(std::move(opened.value()));
    songs_ = std::make_unique<SqliteSongRepository>(*store_);
    sermons_ = std::make_unique<SqliteSermonRepository>(*store_);
    assets_ = std::make_unique<SqliteAssetRepository>(*store_);
    service_orders_ = std::make_unique<SqliteServiceOrderRepository>(*store_);
    content_archive_ = std::make_unique<SqliteContentArchiveRepository>(*store_);
    asset_importer_ = std::make_unique<AssetImporter>(*store_);
    sermon_importer_ = std::make_unique<SermonDeckImporter>(*store_, *sermons());
    root_ = store_->root();
    return domain::success();
}

void ProjectSession::close() noexcept {
    sermon_importer_.reset();
    asset_importer_.reset();
    content_archive_.reset();
    service_orders_.reset();
    assets_.reset();
    sermons_.reset();
    songs_.reset();
    store_.reset();
    root_.clear();
}

SongRepository* ProjectSession::songs() noexcept {
    return songs_.get();
}

const SongRepository* ProjectSession::songs() const noexcept {
    return songs_.get();
}

SermonRepository* ProjectSession::sermons() noexcept {
    return sermons_.get();
}

const SermonRepository* ProjectSession::sermons() const noexcept {
    return sermons_.get();
}

AssetRepository* ProjectSession::assets() noexcept {
    return assets_.get();
}

const AssetRepository* ProjectSession::assets() const noexcept {
    return assets_.get();
}

ServiceOrderRepository* ProjectSession::service_orders() noexcept {
    return service_orders_.get();
}

const ServiceOrderRepository* ProjectSession::service_orders() const noexcept {
    return service_orders_.get();
}

ContentArchiveRepository* ProjectSession::content_archive() noexcept {
    return content_archive_.get();
}

const ContentArchiveRepository* ProjectSession::content_archive() const noexcept {
    return content_archive_.get();
}

}  // namespace shekinah::storage
