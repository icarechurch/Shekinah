#pragma once

#include "shekinah/domain/result.hpp"

#include <filesystem>
#include <memory>

namespace shekinah::storage {

class ProjectStore;
class SqliteContentRepository;
class AssetImporter;
class AssetRepository;
class ContentArchiveRepository;
class SermonRepository;
class SermonDeckImporter;
class ServiceOrderRepository;
class SongRepository;

class ProjectSession final {
public:
    ProjectSession();
    ProjectSession(const ProjectSession&) = delete;
    ProjectSession& operator=(const ProjectSession&) = delete;
    ProjectSession(ProjectSession&&) = delete;
    ProjectSession& operator=(ProjectSession&&) = delete;
    ~ProjectSession();

    domain::Status open(const std::filesystem::path& root);
    void close() noexcept;

    [[nodiscard]] bool is_open() const noexcept { return store_ != nullptr; }
    [[nodiscard]] const std::filesystem::path& root() const noexcept { return root_; }
    [[nodiscard]] ProjectStore* store() noexcept { return store_.get(); }
    [[nodiscard]] const ProjectStore* store() const noexcept { return store_.get(); }
    [[nodiscard]] AssetImporter* asset_importer() noexcept { return asset_importer_.get(); }
    [[nodiscard]] const AssetImporter* asset_importer() const noexcept {
        return asset_importer_.get();
    }
    [[nodiscard]] SermonDeckImporter* sermon_importer() noexcept { return sermon_importer_.get(); }
    [[nodiscard]] const SermonDeckImporter* sermon_importer() const noexcept {
        return sermon_importer_.get();
    }
    [[nodiscard]] SqliteContentRepository* repository() noexcept { return repository_.get(); }
    [[nodiscard]] const SqliteContentRepository* repository() const noexcept {
        return repository_.get();
    }
    [[nodiscard]] SongRepository* songs() noexcept;
    [[nodiscard]] const SongRepository* songs() const noexcept;
    [[nodiscard]] SermonRepository* sermons() noexcept;
    [[nodiscard]] const SermonRepository* sermons() const noexcept;
    [[nodiscard]] AssetRepository* assets() noexcept;
    [[nodiscard]] const AssetRepository* assets() const noexcept;
    [[nodiscard]] ServiceOrderRepository* service_orders() noexcept;
    [[nodiscard]] const ServiceOrderRepository* service_orders() const noexcept;
    [[nodiscard]] ContentArchiveRepository* content_archive() noexcept;
    [[nodiscard]] const ContentArchiveRepository* content_archive() const noexcept;

private:
    std::filesystem::path root_;
    std::unique_ptr<ProjectStore> store_;
    std::unique_ptr<SqliteContentRepository> repository_;
    std::unique_ptr<AssetImporter> asset_importer_;
    std::unique_ptr<SermonDeckImporter> sermon_importer_;
};

}  // namespace shekinah::storage
