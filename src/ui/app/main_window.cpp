#include "shekinah/ui/main_window.hpp"

#include "shekinah/application/song_text_importer.hpp"
#include "shekinah/storage/asset_importer.hpp"
#include "shekinah/storage/content_repository.hpp"
#include "shekinah/storage/project_store.hpp"
#include "shekinah/storage/sermon_deck_importer.hpp"
#include "shekinah/ui/library_dock_widget.hpp"
#include "shekinah/ui/operator_page.hpp"
#include "shekinah/ui/reflow_editor_dialog.hpp"
#include "shekinah/ui/service_order_widget.hpp"

#include <QFileDialog>

#include <fstream>
#include <string>
#include <utility>
#include <vector>

namespace shekinah::ui {

ShekinahWindow::ShekinahWindow(QWidget* parent) : QMainWindow(parent) {
    setObjectName(QStringLiteral("shekinah-window"));
    setWindowTitle(QStringLiteral("Shekinah"));
    resize(1280, 720);

    operator_page_ = new OperatorPage{this};
    setCentralWidget(operator_page_);
    library_dock_ = new LibraryDockWidget{this};
    addDockWidget(Qt::BottomDockWidgetArea, library_dock_);

    library_dock_->set_import_asset_handler([this] { import_asset(); });
    library_dock_->set_import_song_handler([this] { import_song_text(); });
    library_dock_->set_new_song_handler([this] { create_song(); });
    library_dock_->set_import_sermon_handler([this] { import_sermon_deck(); });
    library_dock_->set_add_content_handler([this](domain::ContentRef content) {
        if (!repository_) {
            return;
        }
        const auto appended = repository_->append_to_order(
            domain::ServiceOrderId{"default-service"}, std::move(content));
        if (appended) {
            refresh_service_order();
        }
    });
}

ShekinahWindow::~ShekinahWindow() = default;

domain::Status ShekinahWindow::open_project(const std::filesystem::path& root) {
    auto opened = storage::ProjectStore::create_or_open(root);
    if (!opened) {
        return domain::Status::fail(opened.error());
    }

    store_ = std::make_unique<storage::ProjectStore>(std::move(opened.value()));
    repository_ = std::make_unique<storage::SqliteContentRepository>(*store_);
    project_root_ = root;
    next_content_id_ = 1;

    const auto order = repository_->ensure_service_order(
        domain::ServiceOrderId{"default-service"}, "Service Order");
    if (!order) {
        repository_.reset();
        store_.reset();
        project_root_.clear();
        return order;
    }
    refresh_service_order();
    return domain::success();
}

void ShekinahWindow::refresh_service_order() {
    if (!repository_) {
        operator_page_->set_service_order({});
        return;
    }

    const auto loaded = repository_->load_order(domain::ServiceOrderId{"default-service"});
    if (!loaded) {
        operator_page_->set_service_order({});
        return;
    }

    std::vector<ServiceOrderView> views;
    views.reserve(loaded->size());
    for (const auto& item : *loaded) {
        QString label = QStringLiteral("Unknown content");
        if (item.content.kind == domain::ContentKind::song) {
            const auto content_id = std::get<domain::SongId>(item.content.id);
            const auto song = repository_->load_song(content_id);
            label = song ? QString::fromStdString(song->title)
                         : QStringLiteral("Song %1").arg(QString::fromStdString(content_id.value));
        } else {
            const auto content_id = std::get<domain::SermonId>(item.content.id);
            const auto sermon = repository_->load_deck(content_id);
            label = sermon ? QString::fromStdString(sermon->title)
                           : QStringLiteral("Sermon %1").arg(QString::fromStdString(content_id.value));
        }
        views.push_back(ServiceOrderView{item, std::move(label)});
    }
    operator_page_->set_service_order(std::move(views));
}

void ShekinahWindow::import_asset() {
    if (!store_) {
        return;
    }
    const auto path = QFileDialog::getOpenFileName(
        this, QStringLiteral("Import Asset"), {}, QStringLiteral("Media (*.png *.jpg *.jpeg *.mp4)"));
    if (path.isEmpty()) {
        return;
    }
    const auto imported = import_asset_file(path.toStdString());
    if (imported) {
        library_dock_->add_asset(*imported);
    }
}

domain::Result<domain::AssetRecord> ShekinahWindow::import_asset_file(
    const std::filesystem::path& source) {
    if (!store_) {
        return domain::Result<domain::AssetRecord>::fail(
            {domain::ErrorCode::storage_failure, "Open a project before importing assets"});
    }
    return storage::AssetImporter{*store_}.import_file(source);
}

void ShekinahWindow::import_song_text() {
    if (!repository_) {
        return;
    }
    const auto path = QFileDialog::getOpenFileName(
        this, QStringLiteral("Import Song Text"), {}, QStringLiteral("Text files (*.txt *.lyrics);;All files (*)"));
    if (path.isEmpty()) {
        return;
    }
    const auto imported = import_song_file(path.toStdString());
    if (imported) {
        library_dock_->add_song(*imported);
    }
}

domain::Result<domain::Song> ShekinahWindow::import_song_file(
    const std::filesystem::path& source) {
    if (!repository_) {
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
    song.id = domain::SongId{"song-" + std::to_string(next_content_id_++)};
    const auto saved = repository_->save_song(song);
    if (!saved) {
        return domain::Result<domain::Song>::fail(saved.error());
    }
    return saved;
}

void ShekinahWindow::import_sermon_deck() {
    if (!store_) {
        return;
    }
    const auto paths = QFileDialog::getOpenFileNames(
        this, QStringLiteral("Import Sermon Deck"), {}, QStringLiteral("PNG slides (*.png)"));
    if (paths.isEmpty()) {
        return;
    }
    std::vector<std::filesystem::path> files;
    files.reserve(static_cast<std::size_t>(paths.size()));
    for (const auto& path : paths) {
        files.emplace_back(path.toStdString());
    }
    const auto imported = import_sermon_files(std::move(files), "Imported Sermon");
    if (imported) {
        library_dock_->add_sermon(*imported);
    }
}

domain::Result<domain::SermonDeck> ShekinahWindow::import_sermon_files(
    std::vector<std::filesystem::path> sources, std::string title) {
    if (!store_) {
        return domain::Result<domain::SermonDeck>::fail(
            {domain::ErrorCode::storage_failure, "Open a project before importing sermons"});
    }
    return storage::SermonDeckImporter{*store_}.import_pngs(
        domain::SermonId{"sermon-" + std::to_string(next_content_id_++)}, std::move(title),
        std::move(sources));
}

void ShekinahWindow::create_song() {
    auto* editor = new ReflowEditorDialog{this};
    editor->setAttribute(Qt::WA_DeleteOnClose);
    editor->show();
}

}  // namespace shekinah::ui
