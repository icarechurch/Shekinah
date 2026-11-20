#pragma once

#include <QMainWindow>

#include <filesystem>
#include <memory>
#include <string>
#include <vector>

#include "shekinah/domain/model.hpp"
#include "shekinah/domain/result.hpp"

namespace shekinah::storage {
class ProjectStore;
class SqliteContentRepository;
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
    int next_content_id_{1};

    void refresh_service_order();
    void import_asset();
    void import_song_text();
    void import_sermon_deck();
    void create_song();
};

}  // namespace shekinah::ui
