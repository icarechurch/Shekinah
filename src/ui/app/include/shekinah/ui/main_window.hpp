#pragma once

#include <QMainWindow>

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

namespace shekinah::media {
class AudienceOutput;
}

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
        std::vector<std::filesystem::path> sources, std::string title);
    media::AudienceOutput* audience_output();
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
    int next_content_id_{1};
    application::PresentationService presentation_;

    void configure_operator();
    void refresh_library();
    void refresh_service_order();
    void refresh_cues();
    void update_program_status();
    void import_asset();
    void import_song_text();
    void import_sermon_deck();
    void create_song();
};

}  // namespace shekinah::ui
