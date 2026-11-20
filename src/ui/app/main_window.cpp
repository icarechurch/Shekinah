#include "shekinah/ui/main_window.hpp"

#include "shekinah/application/resource_governor.hpp"
#include "shekinah/application/song_text_importer.hpp"
#include "shekinah/media/audience_output.hpp"
#include "shekinah/media/memory_diagnostics.hpp"
#include "shekinah/storage/asset_importer.hpp"
#include "shekinah/storage/content_repository.hpp"
#include "shekinah/storage/project_store.hpp"
#include "shekinah/storage/sermon_deck_importer.hpp"
#include "shekinah/ui/cue_grid_widget.hpp"
#include "shekinah/ui/library_dock_widget.hpp"
#include "shekinah/ui/operator_page.hpp"
#include "shekinah/ui/program_status_widget.hpp"
#include "shekinah/ui/reflow_editor_dialog.hpp"
#include "shekinah/ui/service_order_widget.hpp"

#include <QFileDialog>
#include <QInputDialog>
#include <QMenu>
#include <QMenuBar>
#include <QStringList>
#include <QTimer>

#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace shekinah::ui {
namespace {

class WindowResourcePools final : public application::ResourcePools {
public:
    void purge_non_live_caches() override {}
    void cancel_queued_work() override {}
    void release_non_active_media() override {}
};

void normalize_song_cue_ids(domain::Song& song) {
    for (std::size_t index = 0; index != song.slides.size(); ++index) {
        song.slides[index].id = domain::CueId{
            song.id.value + "-slide-" + std::to_string(index)};
    }
}

}  // namespace

ShekinahWindow::ShekinahWindow(QWidget* parent) : QMainWindow(parent) {
    setObjectName(QStringLiteral("shekinah-window"));
    setWindowTitle(QStringLiteral("Shekinah"));
    resize(1280, 720);

    resource_pools_ = std::make_unique<WindowResourcePools>();
    resource_governor_ = std::make_unique<application::ResourceGovernor>(*resource_pools_);
    resource_timer_ = new QTimer{this};
    resource_timer_->setInterval(1000);
    QObject::connect(resource_timer_, &QTimer::timeout, this, [this] {
        sample_memory_usage(media::MemoryDiagnostics::working_set_bytes());
    });
    resource_timer_->start();

    operator_page_ = new OperatorPage{this};
    setCentralWidget(operator_page_);
    configure_operator();
    library_dock_ = new LibraryDockWidget{this};
    addDockWidget(Qt::BottomDockWidgetArea, library_dock_);

    auto* window_menu = menuBar()->addMenu(QStringLiteral("Window"));
    auto* audience_action = window_menu->addAction(QStringLiteral("Show Audience Output"));
    audience_action->setObjectName(QStringLiteral("show-audience-output"));
    QObject::connect(audience_action, &QAction::triggered, this, [this] {
        audience_output()->show_movable();
    });

    library_dock_->set_import_asset_handler([this] { import_asset(); });
    library_dock_->set_import_song_handler([this] { import_song_text(); });
    library_dock_->set_new_song_handler([this] { create_song(); });
    library_dock_->set_import_sermon_handler([this] { import_sermon_deck(); });
    library_dock_->set_new_sermon_handler([this] { import_sermon_deck(); });
    library_dock_->set_edit_song_handler([this](domain::SongId id) { edit_song(id); });
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

media::AudienceOutput* ShekinahWindow::audience_output() {
    if (!audience_output_) {
        audience_output_ = std::make_unique<media::AudienceOutput>();
    }
    return audience_output_.get();
}

void ShekinahWindow::sample_memory_usage(std::size_t working_set_bytes) {
    if (resource_governor_) {
        resource_governor_->on_working_set_sample(working_set_bytes);
    }
}

std::size_t ShekinahWindow::last_working_set_bytes() const noexcept {
    return resource_governor_ == nullptr ? 0 : resource_governor_->last_sample_bytes();
}

bool ShekinahWindow::live_media_admitted() const noexcept {
    return resource_governor_ == nullptr || resource_governor_->may_load_live_media();
}

void ShekinahWindow::configure_operator() {
    operator_page_->set_service_order_selection_handler([this](domain::ServiceOrderItemId item_id) {
        if (presentation_.select_service_item(item_id)) {
            refresh_cues();
        }
    });
    operator_page_->set_service_order_move_handler(
        [this](domain::ServiceOrderItemId item_id, std::int64_t target_ordinal) {
            if (repository_ && repository_->move_in_order(item_id, target_ordinal)) {
                refresh_service_order();
            }
        });
    operator_page_->set_take_live_handler([this](domain::CueId cue) {
        const auto selected = presentation_.selected_item();
        if (selected.has_value() && presentation_.take_live(selected->id, cue)) {
            update_program_status();
        }
    });
    operator_page_->set_background_handler([this](domain::CueId cue) { choose_background(cue); });
    operator_page_->set_previous_handler([this] {
        if (presentation_.previous()) {
            update_program_status();
        }
    });
    operator_page_->set_next_handler([this] {
        if (presentation_.next()) {
            update_program_status();
        }
    });
    operator_page_->set_clear_handler([this] {
        presentation_.clear();
        update_program_status();
    });
    operator_page_->set_black_handler([this] {
        presentation_.black();
        update_program_status();
    });
}

domain::Status ShekinahWindow::open_project(const std::filesystem::path& root) {
    repository_.reset();
    store_.reset();
    project_root_.clear();

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
    refresh_library();
    refresh_service_order();
    return domain::success();
}

void ShekinahWindow::refresh_library() {
    library_dock_->clear_contents();
    if (!repository_) {
        return;
    }

    const auto assets = repository_->list_assets();
    if (assets) {
        for (const auto& asset : *assets) {
            library_dock_->add_asset(asset);
        }
    }
    const auto songs = repository_->list_songs();
    if (songs) {
        for (const auto& song : *songs) {
            library_dock_->add_song(song);
        }
    }
    const auto sermons = repository_->list_decks();
    if (sermons) {
        for (const auto& sermon : *sermons) {
            library_dock_->add_sermon(sermon);
        }
    }
}

void ShekinahWindow::refresh_service_order() {
    presentation_.reset();
    if (!repository_) {
        operator_page_->set_service_order({});
        operator_page_->cue_grid()->set_cues({});
        update_program_status();
        return;
    }

    const auto loaded = repository_->load_order(domain::ServiceOrderId{"default-service"});
    if (!loaded) {
        operator_page_->set_service_order({});
        operator_page_->cue_grid()->set_cues({});
        update_program_status();
        return;
    }

    std::vector<ServiceOrderView> views;
    views.reserve(loaded->size());
    for (const auto& item : *loaded) {
        QString label = QStringLiteral("Unknown content");
        if (item.content.kind == domain::ContentKind::song) {
            const auto content_id = std::get<domain::SongId>(item.content.id);
            const auto song = repository_->load_song(content_id);
            if (song) {
                presentation_.register_song(*song);
                label = QString::fromStdString(song->title);
            } else {
                label = QStringLiteral("Song %1").arg(QString::fromStdString(content_id.value));
            }
        } else {
            const auto content_id = std::get<domain::SermonId>(item.content.id);
            const auto sermon = repository_->load_deck(content_id);
            if (sermon) {
                presentation_.register_sermon(*sermon);
                label = QString::fromStdString(sermon->title);
            } else {
                label = QStringLiteral("Sermon %1").arg(QString::fromStdString(content_id.value));
            }
        }
        views.push_back(ServiceOrderView{item, std::move(label)});
    }
    presentation_.set_service_order(*loaded);
    operator_page_->set_service_order(std::move(views));
    if (!loaded->empty()) {
        operator_page_->service_order_widget()->select_row(0);
    } else {
        operator_page_->cue_grid()->set_cues({});
    }
    update_program_status();
}

void ShekinahWindow::refresh_cues() {
    const auto selected = presentation_.selected_item();
    if (!selected.has_value() || !repository_) {
        operator_page_->cue_grid()->set_cues({});
        return;
    }

    std::vector<CueView> cues;
    if (selected->content.kind == domain::ContentKind::song) {
        const auto song = repository_->load_song(std::get<domain::SongId>(selected->content.id));
        if (!song) {
            operator_page_->cue_grid()->set_cues({});
            return;
        }
        cues.reserve(song->slides.size());
        for (const auto& slide : song->slides) {
            std::ostringstream summary;
            for (std::size_t index = 0; index != slide.lines.size(); ++index) {
                if (index != 0) {
                    summary << '\n';
                }
                summary << slide.lines[index];
            }
            cues.push_back(CueView{
                slide.id,
                QString::fromStdString(slide.section),
                QString::fromStdString(summary.str()),
            });
        }
    } else {
        const auto sermon = repository_->load_deck(std::get<domain::SermonId>(selected->content.id));
        if (!sermon) {
            operator_page_->cue_grid()->set_cues({});
            return;
        }
        cues.reserve(sermon->slides.size());
        for (std::size_t index = 0; index != sermon->slides.size(); ++index) {
            cues.push_back(CueView{
                sermon->slides[index].id,
                QStringLiteral("Slide %1").arg(static_cast<qulonglong>(index + 1)),
                QString::fromStdString(sermon->slides[index].image.value),
            });
        }
    }
    operator_page_->cue_grid()->set_cues(std::move(cues));
}

void ShekinahWindow::update_program_status() {
    operator_page_->program_status()->set_program(presentation_.program());
    if (audience_output_) {
        audience_output_->present(presentation_.program());
        present_live_media();
    }
}

void ShekinahWindow::present_live_media() {
    if (!audience_output_ || !repository_) {
        return;
    }
    const auto& program = presentation_.program();
    if (program.mode != domain::ProgramMode::cue || !program.live_cue.has_value()) {
        audience_output_->clear_slide_image();
        audience_output_->clear_loop_video();
        return;
    }

    std::optional<domain::AssetId> asset_id;
    const auto& cue = *program.live_cue;
    if (cue.content.kind == domain::ContentKind::song) {
        const auto background = presentation_.resolve_background(cue.content, cue.cue);
        if (background && background->has_value()) {
            asset_id = background->value().asset;
        }
    } else {
        const auto deck = repository_->load_deck(std::get<domain::SermonId>(cue.content.id));
        if (deck) {
            const auto slide = std::find_if(deck->slides.begin(), deck->slides.end(), [&](const auto& candidate) {
                return candidate.id == cue.cue;
            });
            if (slide != deck->slides.end()) {
                asset_id = slide->image;
            }
        }
    }

    if (!asset_id.has_value()) {
        audience_output_->clear_slide_image();
        audience_output_->clear_loop_video();
        return;
    }
    const auto asset = repository_->load_asset(*asset_id);
    if (!asset) {
        return;
    }
    const auto path = store_->root() / asset->relative_path;
    if (asset->kind == domain::AssetKind::image) {
        audience_output_->clear_loop_video();
        audience_output_->load_slide_image(path);
    } else {
        if (!live_media_admitted()) {
            return;
        }
        audience_output_->clear_slide_image();
        audience_output_->load_loop_video(path);
    }
}

void ShekinahWindow::choose_background(const domain::CueId& cue) {
    if (!repository_) {
        return;
    }
    const auto assets = repository_->list_assets();
    if (!assets || assets->empty()) {
        return;
    }
    QStringList labels;
    labels.reserve(static_cast<qsizetype>(assets->size()));
    for (const auto& asset : *assets) {
        labels.push_back(QString::fromStdString(asset.relative_path));
    }
    bool accepted = false;
    const auto selected = QInputDialog::getItem(
        this, QStringLiteral("Set Cue Background"), QStringLiteral("Asset"), labels, 0, false,
        &accepted);
    if (!accepted) {
        return;
    }
    const auto index = labels.indexOf(selected);
    if (index >= 0) {
        set_slide_background(cue, assets->at(static_cast<std::size_t>(index)).id);
    }
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
    song.id = domain::SongId{allocate_song_id()};
    normalize_song_cue_ids(song);
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
        domain::SermonId{allocate_sermon_id()}, std::move(title),
        std::move(sources));
}

domain::Status ShekinahWindow::set_slide_background(
    const domain::CueId& cue, const domain::AssetId& asset) {
    if (!repository_) {
        return domain::Status::fail(
            {domain::ErrorCode::storage_failure, "Open a project before setting a background"});
    }
    const auto selected = presentation_.selected_item();
    if (!selected.has_value() || selected->content.kind != domain::ContentKind::song) {
        return domain::Status::fail(
            {domain::ErrorCode::invalid_input, "Only selected Song cues accept editable backgrounds"});
    }
    const auto asset_record = repository_->load_asset(asset);
    if (!asset_record) {
        return domain::Status::fail(asset_record.error());
    }
    auto song = repository_->load_song(std::get<domain::SongId>(selected->content.id));
    if (!song) {
        return domain::Status::fail(song.error());
    }
    const auto slide = std::find_if(song->slides.begin(), song->slides.end(), [&](const auto& candidate) {
        return candidate.id == cue;
    });
    if (slide == song->slides.end()) {
        return domain::Status::fail({domain::ErrorCode::not_found, "Song cue was not found"});
    }
    slide->background = domain::BackgroundRef{asset_record->id, domain::FitMode::cover};
    const auto saved = repository_->save_song(*song);
    if (!saved) {
        return domain::Status::fail(saved.error());
    }
    presentation_.register_song(*saved);
    if (presentation_.program().live_cue.has_value() &&
        presentation_.program().live_cue->cue == cue) {
        update_program_status();
    }
    return domain::success();
}

std::string ShekinahWindow::allocate_song_id() {
    for (;;) {
        const auto candidate = "song-" + std::to_string(next_content_id_++);
        if (!repository_ || !repository_->load_song(domain::SongId{candidate})) {
            return candidate;
        }
    }
}

std::string ShekinahWindow::allocate_sermon_id() {
    for (;;) {
        const auto candidate = "sermon-" + std::to_string(next_content_id_++);
        if (!repository_ || !repository_->load_deck(domain::SermonId{candidate})) {
            return candidate;
        }
    }
}

void ShekinahWindow::create_song() {
    if (!repository_) {
        return;
    }
    auto* editor = new ReflowEditorDialog{this};
    editor->setAttribute(Qt::WA_DeleteOnClose);
    editor->load_song(domain::SongId{allocate_song_id()});
    editor->set_title(QStringLiteral("Untitled Song"));
    QObject::connect(editor, &QDialog::accepted, this, [this, editor] {
        const auto draft = editor->draft_song();
        if (!draft || !repository_) {
            return;
        }
        auto song = *draft;
        normalize_song_cue_ids(song);
        const auto saved = repository_->save_song(song);
        if (saved) {
            library_dock_->add_song(*saved);
        }
    });
    editor->show();
}

void ShekinahWindow::edit_song(const domain::SongId& id) {
    if (!repository_) {
        return;
    }
    const auto existing = repository_->load_song(id);
    const auto reflow = repository_->load_song_reflow(id);
    if (!existing || !reflow) {
        return;
    }

    auto* editor = new ReflowEditorDialog{this};
    editor->setAttribute(Qt::WA_DeleteOnClose);
    editor->load_song(existing->id);
    editor->set_title(QString::fromStdString(existing->title));
    editor->set_reflow_document(*reflow);
    QObject::connect(editor, &QDialog::accepted, this, [this, editor, existing] {
        const auto draft = editor->draft_song();
        if (!draft || !repository_) {
            return;
        }
        auto song = *draft;
        song.default_background = existing->default_background;
        for (std::size_t index = 0; index != song.slides.size(); ++index) {
            if (index < existing->slides.size()) {
                song.slides[index].id = existing->slides[index].id;
                song.slides[index].background = existing->slides[index].background;
            } else {
                song.slides[index].id = domain::CueId{
                    song.id.value + "-slide-" + std::to_string(index)};
            }
        }
        const auto saved = repository_->save_song(song);
        if (saved) {
            refresh_library();
            presentation_.register_song(*saved);
            refresh_cues();
            update_program_status();
        }
    });
    editor->show();
}

}  // namespace shekinah::ui
