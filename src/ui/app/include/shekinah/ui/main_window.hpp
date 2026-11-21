#pragma once

#include <QMainWindow>
#include <QString>

#include <cstddef>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

#include "shekinah/application/presentation_service.hpp"
#include "shekinah/domain/model.hpp"
#include "shekinah/domain/result.hpp"

namespace shekinah::storage {
class ProjectStore;
class SqliteContentRepository;
}

namespace shekinah::application {
class ResourceGovernor;
class ResourcePools;
}

namespace shekinah::media {
class AudienceOutput;
}

class QTimer;

namespace shekinah::ui {

class LibraryDockWidget;
class OperatorPage;

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
    [[nodiscard]] bool project_open() const noexcept { return store_ != nullptr; }
    [[nodiscard]] const std::filesystem::path& project_root() const noexcept { return project_root_; }

    [[nodiscard]] OperatorPage* operator_page() const noexcept { return operator_page_; }
    [[nodiscard]] LibraryDockWidget* library_dock() const noexcept { return library_dock_; }

private:
    OperatorPage* operator_page_{nullptr};
    LibraryDockWidget* library_dock_{nullptr};
    std::filesystem::path project_root_;
    std::unique_ptr<storage::ProjectStore> store_;
    std::unique_ptr<storage::SqliteContentRepository> repository_;
    std::unique_ptr<media::AudienceOutput> audience_output_;
    std::unique_ptr<application::ResourcePools> resource_pools_;
    std::unique_ptr<application::ResourceGovernor> resource_governor_;
    QTimer* resource_timer_{nullptr};
    int next_content_id_{1};
    application::PresentationService presentation_;

    void configure_operator();
    void refresh_library();
    void refresh_service_order();
    void refresh_cues();
    void update_program_status();
    void present_live_media();
    QString asset_name_for_cue(const domain::CueId& cue) const;
    void apply_asset_to_song(domain::ContentRef content, domain::AssetId asset);
    void apply_asset_to_slide(domain::CueId cue, domain::AssetId asset);
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
