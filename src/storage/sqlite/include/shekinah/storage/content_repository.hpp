#pragma once

#include "shekinah/storage/asset_repository.hpp"
#include "shekinah/storage/content_archive_repository.hpp"
#include "shekinah/storage/sermon_repository.hpp"
#include "shekinah/storage/service_order_repository.hpp"
#include "shekinah/storage/song_repository.hpp"

#include <memory>

namespace shekinah::storage {

class ProjectStore;
class SqliteSongRepository;
class SqliteSermonRepository;
class SqliteAssetRepository;
class SqliteServiceOrderRepository;
class SqliteContentArchiveRepository;

class SqliteContentRepository final : public SongRepository,
                                      public SermonRepository,
                                      public AssetRepository,
                                      public ServiceOrderRepository,
                                      public ContentArchiveRepository {
public:
    explicit SqliteContentRepository(ProjectStore& store);
    ~SqliteContentRepository() override;

    domain::Result<domain::Song> save_song(const domain::Song& song) override;
    domain::Result<domain::Song> load_song(const domain::SongId& id) const override;
    domain::Result<application::SongReflowDocument> load_song_reflow(
        const domain::SongId& id) const override;
    domain::Result<std::vector<domain::Song>> list_songs() const override;
    domain::Result<std::size_t> song_count() const override;
    domain::Result<domain::SermonDeck> save_deck(const domain::SermonDeck& deck) override;
    domain::Result<domain::SermonDeck> load_deck(const domain::SermonId& id) const override;
    domain::Result<std::vector<domain::SermonDeck>> list_decks() const override;
    domain::Result<std::vector<domain::AssetRecord>> list_assets() const override;
    domain::Result<domain::AssetRecord> load_asset(const domain::AssetId& id) const override;
    domain::Status archive_song(const domain::SongId& id) override;
    domain::Status archive_deck(const domain::SermonId& id) override;
    domain::Status ensure_service_order(const domain::ServiceOrderId& id,
                                        std::string title) override;
    domain::Result<domain::ServiceOrderItem> append_to_order(
        const domain::ServiceOrderId& order_id, domain::ContentRef content) override;
    domain::Status remove_from_order(const domain::ServiceOrderItemId& item_id) override;
    domain::Status move_in_order(const domain::ServiceOrderItemId& item_id,
                                 std::int64_t target_ordinal) override;
    domain::Result<std::vector<domain::ServiceOrderItem>> load_order(
        const domain::ServiceOrderId& order_id) const override;

private:
    ProjectStore& store_;
    std::unique_ptr<SqliteSongRepository> songs_;
    std::unique_ptr<SqliteSermonRepository> sermons_;
    std::unique_ptr<SqliteAssetRepository> assets_;
    std::unique_ptr<SqliteServiceOrderRepository> service_orders_;
    std::unique_ptr<SqliteContentArchiveRepository> archive_;
};

}  // namespace shekinah::storage
