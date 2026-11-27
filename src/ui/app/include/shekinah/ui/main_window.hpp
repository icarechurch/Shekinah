#pragma once

#include <QMainWindow>
#include <QString>

#include <cstddef>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "shekinah/application/presentation_service.hpp"
#include "shekinah/domain/model.hpp"
#include "shekinah/domain/result.hpp"
#include "shekinah/storage/project_session.hpp"
#include "shekinah/ui/support/theme.hpp"

namespace shekinah::application {
class ResourceGovernor;
class ResourcePools;
}

namespace shekinah::media {
class AudienceOutput;
}

class QActionGroup;
class QTimer;

namespace shekinah::storage {
class AssetRepository;
class AssetImporter;
class ContentArchiveRepository;
class SermonRepository;
class SermonDeckImporter;
class ServiceOrderRepository;
class SongRepository;
}

namespace shekinah::ui {

namespace main_window_internal {
struct LiveMediaSnapshot;
class LiveMediaCoordinator;
class LiveOutputCoordinator;
}

struct LiveSlideDetails;

class LibraryDockWidget;
class LibraryCatalogCoordinator;
class OperatorPage;
class ContentPersistenceCoordinator;
class CuePresentationCoordinator;
class AssetSlideInsertionCoordinator;
class SongAssetCoordinator;
class SlideMutationCoordinator;
class ServiceOrderCoordinator;
class ServiceOrderViewCoordinator;
class ProjectLifecycleCoordinator;
class WindowResourcePools;

class ShekinahWindow final : public QMainWindow {
public:
    explicit ShekinahWindow(QWidget* parent = nullptr);
    ~ShekinahWindow() override;
    domain::Status open_project(const std::filesystem::path& root);
    domain::Result<domain::AssetRecord> import_asset_file(const std::filesystem::path& source);
    domain::Result<domain::Song> import_song_file(const std::filesystem::path& source);
    domain::Result<domain::SermonDeck> import_sermon_files(
        std::vector<std::filesystem::path> sources, std::string title, bool preserve_order = false);
    domain::Status set_slide_background(const domain::CueId& cue, const domain::AssetId& asset);
    media::AudienceOutput* audience_output();
    void sample_memory_usage(std::size_t working_set_bytes);
    [[nodiscard]] std::size_t last_working_set_bytes() const noexcept;
    [[nodiscard]] bool live_media_admitted() const noexcept;
    [[nodiscard]] bool project_open() const noexcept { return session_.is_open(); }
    [[nodiscard]] const std::filesystem::path& project_root() const noexcept { return session_.root(); }

    [[nodiscard]] OperatorPage* operator_page() const noexcept { return operator_page_; }
    [[nodiscard]] LibraryDockWidget* library_dock() const noexcept { return library_dock_; }
    void set_theme(ThemeId id);
    [[nodiscard]] ThemeId theme() const noexcept;

private:
    OperatorPage* operator_page_{nullptr};
    LibraryDockWidget* library_dock_{nullptr};
    storage::ProjectSession session_;
    std::unique_ptr<media::AudienceOutput> audience_output_;
    std::unique_ptr<WindowResourcePools> resource_pools_;
    std::unique_ptr<application::ResourceGovernor> resource_governor_;
    std::unique_ptr<ContentPersistenceCoordinator> content_persistence_coordinator_;
    std::unique_ptr<AssetSlideInsertionCoordinator> asset_slide_insertion_coordinator_;
    std::unique_ptr<SongAssetCoordinator> song_asset_coordinator_;
    std::unique_ptr<SlideMutationCoordinator> slide_mutation_coordinator_;
    std::unique_ptr<CuePresentationCoordinator> cue_presentation_coordinator_;
    std::unique_ptr<main_window_internal::LiveMediaCoordinator> live_media_coordinator_;
    std::unique_ptr<main_window_internal::LiveOutputCoordinator> live_output_coordinator_;
    std::unique_ptr<LibraryCatalogCoordinator> library_catalog_coordinator_;
    std::unique_ptr<ProjectLifecycleCoordinator> project_lifecycle_coordinator_;
    std::unique_ptr<ServiceOrderCoordinator> service_order_coordinator_;
    std::unique_ptr<ServiceOrderViewCoordinator> service_order_view_coordinator_;
    QTimer* resource_timer_{nullptr};
    QActionGroup* theme_actions_{nullptr};
    int next_content_id_{1};
    application::PresentationService presentation_;

    void apply_theme();
    void sync_theme_actions();
    void configure_window_actions();
    void configure_window_menu();
    void configure_file_menu();
    void configure_presentation_menu();
    void configure_view_menu();
    void configure_feature_handlers();
    void configure_library_handlers();
    void configure_operator_feature_handlers();
    [[nodiscard]] storage::AssetImporter* asset_importer() noexcept {
        return session_.asset_importer();
    }
    [[nodiscard]] const storage::AssetImporter* asset_importer() const noexcept {
        return session_.asset_importer();
    }
    [[nodiscard]] storage::SermonDeckImporter* sermon_importer() noexcept {
        return session_.sermon_importer();
    }
    [[nodiscard]] const storage::SermonDeckImporter* sermon_importer() const noexcept {
        return session_.sermon_importer();
    }
    [[nodiscard]] storage::SongRepository* songs() noexcept { return session_.songs(); }
    [[nodiscard]] const storage::SongRepository* songs() const noexcept { return session_.songs(); }
    [[nodiscard]] storage::SermonRepository* sermons() noexcept { return session_.sermons(); }
    [[nodiscard]] const storage::SermonRepository* sermons() const noexcept {
        return session_.sermons();
    }
    [[nodiscard]] storage::AssetRepository* assets() noexcept { return session_.assets(); }
    [[nodiscard]] const storage::AssetRepository* assets() const noexcept { return session_.assets(); }
    void configure_operator();
    void configure_operator_output_handlers();
    void refresh_library();
    void refresh_service_order();
    void close_project();
    void refresh_cues();
    void update_program_status();
    void present_live_media();
    QString asset_name_for_cue(const domain::CueId& cue) const;
    void remove_asset_from_song(const domain::CueId& cue);
    void apply_asset_to_song(domain::ContentRef content, domain::AssetId asset);
    void apply_asset_to_slide(domain::CueId cue, domain::AssetId asset);
    void insert_asset_at_slide_position(std::size_t index, domain::AssetId asset);
    void choose_background(const domain::CueId& cue);
    void import_asset();
    void import_song_text();
    void import_sermon_deck();
    void create_sermon();
    void edit_sermon(const domain::SermonId& id);
    void create_song();
    void edit_song(const domain::SongId& id);
    void show_inspector(const domain::CueId& cue);
    void save_inspected_slide(domain::CueId cue,
                              std::string section,
                              std::vector<std::string> lines,
                              domain::FitMode fit);
    void duplicate_slide(const domain::CueId& cue);
    void delete_slide(const domain::CueId& cue);
    std::string allocate_song_id();
    std::string allocate_sermon_id();
};

}  // namespace shekinah::ui
