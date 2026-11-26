#include "shekinah/ui/main_window.hpp"

#include "shekinah/application/cue_presenter.hpp"
#include "shekinah/application/live_slide_presenter.hpp"
#include "shekinah/application/resource_governor.hpp"
#include "shekinah/application/service_order_presenter.hpp"
#include "shekinah/application/slide_insertion.hpp"
#include "shekinah/application/song_backgrounds.hpp"
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
#include "shekinah/ui/sermon_editor_dialog.hpp"
#include "shekinah/ui/service_order_widget.hpp"
#include "shekinah/ui/support/app_icon.hpp"

#include <QActionGroup>
#include <QFileDialog>
#include <QInputDialog>
#include <QKeySequence>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QScreen>
#include <QStringList>
#include <QTimer>

#include <algorithm>
#include <fstream>
#include <string>
#include <utility>
#include <vector>

namespace shekinah::ui {
namespace {

class WindowResourcePools final : public application::ResourcePools {
public:
    explicit WindowResourcePools(std::function<void()> purge_callback = {})
        : purge_callback_(std::move(purge_callback)) {}

    void set_purge_callback(std::function<void()> purge_callback) {
        purge_callback_ = std::move(purge_callback);
    }

    void purge_non_live_caches() override {
        if (purge_callback_) {
            purge_callback_();
        }
    }
    void cancel_queued_work() override {}
    void release_non_active_media() override {}

private:
    std::function<void()> purge_callback_;
};

void normalize_song_cue_ids(domain::Song& song) {
    for (std::size_t index = 0; index != song.slides.size(); ++index) {
        song.slides[index].id = domain::CueId{
            song.id.value + "-slide-" + std::to_string(index)};
    }
}

application::SongReflowDocument reflow_document_for_song(const domain::Song& song) {
    std::string text;
    std::vector<application::SlideBreak> breaks;
    for (std::size_t index = 0; index != song.slides.size(); ++index) {
        if (index != 0) {
            breaks.push_back(application::SlideBreak{text.size(), song.slides[index].section});
            text.push_back('\n');
        }
        for (std::size_t line = 0; line != song.slides[index].lines.size(); ++line) {
            if (line != 0) {
                text.push_back('\n');
            }
            text += song.slides[index].lines[line];
        }
    }
    return application::SongReflowDocument{std::move(text), std::move(breaks)};
}

}  // namespace

ShekinahWindow::ShekinahWindow(QWidget* parent) : QMainWindow(parent) {
    setObjectName(QStringLiteral("shekinah-window"));
    setWindowTitle(QStringLiteral("Shekinah"));
    setWindowIcon(app_icon());
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
    static_cast<WindowResourcePools*>(resource_pools_.get())->set_purge_callback([this] {
        if (library_dock_ != nullptr) {
            library_dock_->purge_non_live_caches();
        }
    });
    addDockWidget(Qt::BottomDockWidgetArea, library_dock_);

    auto* window_menu = menuBar()->addMenu(QStringLiteral("Window"));
    auto* audience_action = window_menu->addAction(QStringLiteral("Show Audience Output"));
    audience_action->setObjectName(QStringLiteral("show-audience-output"));
    QObject::connect(audience_action, &QAction::triggered, this, [this] {
        audience_output()->show_movable();
        operator_page_->program_status()->set_output_status(
            QStringLiteral("Output: Window"), QStringLiteral("Audience window"), true);
    });

    library_dock_->set_import_asset_handler([this] { import_asset(); });
    library_dock_->set_import_song_handler([this] { import_song_text(); });
    library_dock_->set_new_song_handler([this] { create_song(); });
    library_dock_->set_import_sermon_handler([this] { import_sermon_deck(); });
    library_dock_->set_new_sermon_handler([this] { create_sermon(); });
    library_dock_->set_edit_sermon_handler([this](domain::SermonId id) { edit_sermon(id); });
    library_dock_->set_edit_song_handler([this](domain::SongId id) { edit_song(id); });
    operator_page_->set_authoring_handlers(
        [this] { create_song(); },
        [this] { import_song_text(); },
        [this] { import_sermon_deck(); },
        [this] { import_asset(); });
    operator_page_->set_service_order_new_sermon_handler([this] { create_sermon(); });
    operator_page_->set_service_order_edit_handler([this](domain::ContentRef content) {
        if (content.kind == domain::ContentKind::song) {
            edit_song(std::get<domain::SongId>(content.id));
        } else {
            edit_sermon(std::get<domain::SermonId>(content.id));
        }
    });
    operator_page_->set_inspector_handler([this](domain::CueId cue) { show_inspector(cue); });
    operator_page_->set_edit_slide_handler([this](domain::CueId cue) { show_inspector(cue); });
    operator_page_->set_cue_asset_name_handler(
        [this](domain::CueId cue) { return asset_name_for_cue(cue); });
    operator_page_->set_remove_asset_handler(
        [this](domain::CueId cue) { remove_asset_from_song(cue); });
    operator_page_->set_duplicate_slide_handler([this](domain::CueId cue) { duplicate_slide(cue); });
    operator_page_->set_delete_slide_handler([this](domain::CueId cue) { delete_slide(cue); });
    operator_page_->set_service_order_asset_drop_handler(
        [this](domain::ContentRef content, domain::AssetId asset) {
            apply_asset_to_song(std::move(content), std::move(asset));
        });
    operator_page_->set_service_order_content_drop_handler([this](domain::ContentRef content) {
        if (!repository()) {
            return;
        }
        const auto appended = repository()->append_to_order(
            domain::ServiceOrderId{"default-service"}, std::move(content));
        if (appended) {
            refresh_service_order();
        }
    });
    operator_page_->set_service_order_remove_handler([this](domain::ServiceOrderItemId item) {
        if (repository() && repository()->remove_from_order(item)) {
            refresh_service_order();
        }
    });
    operator_page_->set_cue_asset_drop_handler(
        [this](domain::CueId cue, domain::AssetId asset) {
            apply_asset_to_slide(std::move(cue), std::move(asset));
        });
    operator_page_->set_cue_asset_insert_handler(
        [this](std::size_t index, domain::AssetId asset) {
            insert_asset_at_slide_position(index, std::move(asset));
        });
    operator_page_->set_inspector_save_handler(
        [this](domain::CueId cue, std::string section, std::vector<std::string> lines, domain::FitMode fit) {
            save_inspected_slide(std::move(cue), std::move(section), std::move(lines), fit);
        });
    library_dock_->setMinimumHeight(120);
    library_dock_->show();
    resizeDocks({library_dock_}, {190}, Qt::Vertical);
    const auto add_content_to_presentation = [this](domain::ContentRef content) {
        if (!repository()) {
            return;
        }
        const auto appended = repository()->append_to_order(
            domain::ServiceOrderId{"default-service"}, std::move(content));
        if (appended) {
            refresh_service_order();
        }
    };
    library_dock_->set_add_content_handler(add_content_to_presentation);

    auto* file_menu = menuBar()->addMenu(QStringLiteral("File"));
    auto* new_project_action = file_menu->addAction(QStringLiteral("New Project"));
    new_project_action->setObjectName(QStringLiteral("new-project"));
    new_project_action->setShortcut(QKeySequence::New);
    QObject::connect(new_project_action, &QAction::triggered, this, [this] {
        const auto root = QFileDialog::getExistingDirectory(this, QStringLiteral("Create or Open Project"));
        if (!root.isEmpty()) {
            open_project(root.toStdString());
        }
    });
    auto* open_project_action = file_menu->addAction(QStringLiteral("Open Project"));
    open_project_action->setObjectName(QStringLiteral("open-project"));
    open_project_action->setShortcut(QKeySequence::Open);
    QObject::connect(open_project_action, &QAction::triggered, this, [this] {
        const auto root = QFileDialog::getExistingDirectory(this, QStringLiteral("Open Project"));
        if (!root.isEmpty()) {
            open_project(root.toStdString());
        }
    });
    auto* close_project_action = file_menu->addAction(QStringLiteral("Close Project"));
    close_project_action->setObjectName(QStringLiteral("close-project"));
    QObject::connect(close_project_action, &QAction::triggered, this, [this] {
        session_.close();
        presentation_.reset();
        refresh_library();
        refresh_service_order();
    });

    auto* presentation_menu = menuBar()->addMenu(QStringLiteral("Presentation"));
    const auto add_presentation_action = [&](const QString& label,
                                             const QString& object_name,
                                             const QKeySequence& shortcut,
                                             auto handler) {
        auto* action = presentation_menu->addAction(label);
        action->setObjectName(object_name);
        action->setShortcut(shortcut);
        action->setShortcutContext(Qt::WindowShortcut);
        QObject::connect(action, &QAction::triggered, this, handler);
    };
    add_presentation_action(QStringLiteral("Previous Cue"), QStringLiteral("previous-cue"),
                            QKeySequence{Qt::Key_PageUp}, [this] {
                                if (presentation_.previous()) {
                                    update_program_status();
                                }
                            });
    add_presentation_action(QStringLiteral("Next Cue"), QStringLiteral("next-cue"),
                            QKeySequence{Qt::Key_PageDown}, [this] {
                                if (presentation_.next()) {
                                    update_program_status();
                                }
                            });
    add_presentation_action(QStringLiteral("Clear Program"), QStringLiteral("clear-program"),
                            QKeySequence{Qt::Key_Escape}, [this] {
                                presentation_.clear();
                                update_program_status();
                            });
    add_presentation_action(QStringLiteral("Blackout"), QStringLiteral("blackout"),
                            QKeySequence{Qt::Key_B}, [this] {
                                presentation_.black();
                                update_program_status();
                            });

    auto* view_menu = menuBar()->addMenu(QStringLiteral("View"));
    view_menu->setObjectName(QStringLiteral("view-menu"));
    auto* theme_menu = view_menu->addMenu(QStringLiteral("Theme"));
    theme_menu->setObjectName(QStringLiteral("theme-menu"));
    theme_actions_ = new QActionGroup{this};
    theme_actions_->setExclusive(true);
    for (const auto& info : ThemeManager::available_themes()) {
        auto* action = theme_menu->addAction(info.label);
        action->setObjectName(QStringLiteral("theme-%1").arg(info.key));
        action->setCheckable(true);
        action->setData(static_cast<int>(info.id));
        action->setStatusTip(info.description);
        theme_actions_->addAction(action);
        QObject::connect(action, &QAction::triggered, this, [this, id = info.id] {
            set_theme(id);
        });
        if (info.id == ThemeId::system) {
            theme_menu->addSeparator();
        }
    }

    apply_theme();
    sync_theme_actions();
    QObject::connect(&ThemeManager::instance(), &ThemeManager::theme_changed, this,
                     [this](ThemeId) {
                         apply_theme();
                         sync_theme_actions();
                     });
}

ShekinahWindow::~ShekinahWindow() = default;

media::AudienceOutput* ShekinahWindow::audience_output() {
    if (!audience_output_) {
        audience_output_ = std::make_unique<media::AudienceOutput>();
        audience_output_->setWindowIcon(app_icon());
        audience_output_->present(presentation_.program());
        present_live_media();
    }
    return audience_output_.get();
}

void ShekinahWindow::sample_memory_usage(std::size_t working_set_bytes) {
    if (resource_governor_) {
        resource_governor_->on_working_set_sample(working_set_bytes);
    }
    if (operator_page_ != nullptr && operator_page_->program_status() != nullptr) {
        operator_page_->program_status()->set_memory_status(
            last_working_set_bytes(), live_media_admitted());
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
            if (repository() && repository()->move_in_order(item_id, target_ordinal)) {
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
    operator_page_->set_show_screen_handler([this](QScreen* screen) {
        if (screen == nullptr) {
            return;
        }
        audience_output()->show_fullscreen(screen);
        operator_page_->program_status()->set_output_status(
            QStringLiteral("Output: %1").arg(screen->name()),
            QStringLiteral("Audience full-screen"),
            true);
    });
    operator_page_->set_show_movable_handler([this] {
        audience_output()->show_movable();
        operator_page_->program_status()->set_output_status(
            QStringLiteral("Output: Window"),
            QStringLiteral("Audience window"),
            true);
    });
    operator_page_->set_hide_output_handler([this] {
        if (audience_output_) {
            audience_output_->hide();
        }
        operator_page_->program_status()->set_output_status(
            QString{},
            QStringLiteral("Output hidden"),
            false);
    });
    operator_page_->set_inspect_live_handler([this] {
        const auto& program = presentation_.program();
        if (program.live_cue.has_value()) {
            show_inspector(program.live_cue->cue);
        }
    });
    operator_page_->set_choose_background_handler([this] {
        const auto& program = presentation_.program();
        if (program.live_cue.has_value()) {
            choose_background(program.live_cue->cue);
        }
    });
}

domain::Status ShekinahWindow::open_project(const std::filesystem::path& root) {
    const auto opened = session_.open(root);
    if (!opened) {
        return opened;
    }

    next_content_id_ = 1;

    const auto order = session_.repository()->ensure_service_order(
        domain::ServiceOrderId{"default-service"}, "Service Order");
    if (!order) {
        session_.close();
        return order;
    }
    refresh_library();
    refresh_service_order();
    return domain::success();
}

void ShekinahWindow::import_song_text() {
    if (!repository()) {
        return;
    }
    const auto path = QFileDialog::getOpenFileName(
        this, QStringLiteral("Import Song Text"), {}, QStringLiteral("Text files (*.txt *.lyrics);;All files (*)"));
    if (path.isEmpty()) {
        return;
    }
    std::ifstream input(path.toStdString(), std::ios::binary);
    if (!input) {
        return;
    }
    const std::string text{std::istreambuf_iterator<char>{input}, std::istreambuf_iterator<char>{}};
    const auto parsed = application::SongTextImporter::parse(text);
    if (!parsed) {
        return;
    }
    auto imported = parsed.value();
    imported.id = domain::SongId{allocate_song_id()};
    normalize_song_cue_ids(imported);

    auto* editor = new ReflowEditorDialog{this};
    editor->setAttribute(Qt::WA_DeleteOnClose);
    editor->load_song(imported.id);
    editor->set_title(QString::fromStdString(imported.title));
    editor->set_reflow_document(reflow_document_for_song(imported));
    QObject::connect(editor, &QDialog::accepted, this, [this, editor] {
        const auto draft = editor->draft_song();
        if (!draft || !repository()) {
            return;
        }
        auto song = *draft;
        normalize_song_cue_ids(song);
        const auto saved = repository()->save_song(song);
        if (saved) {
            library_dock_->add_song(*saved);
        }
    });
    editor->show();
}

domain::Result<domain::Song> ShekinahWindow::import_song_file(
    const std::filesystem::path& source) {
    if (!repository()) {
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
    const auto saved = repository()->save_song(song);
    if (!saved) {
        return domain::Result<domain::Song>::fail(saved.error());
    }
    return saved;
}

void ShekinahWindow::import_sermon_deck() {
    if (!store()) {
        return;
    }
    const auto paths = QFileDialog::getOpenFileNames(
        this, QStringLiteral("Import Sermon Deck"), {},
        QStringLiteral(
            "Media (*.png *.jpg *.jpeg *.webp *.bmp *.gif *.tif *.tiff *.mp4);;All files (*)"));
    if (paths.isEmpty()) {
        return;
    }
    std::vector<std::filesystem::path> files;
    files.reserve(static_cast<std::size_t>(paths.size()));
    for (const auto& path : paths) {
        files.emplace_back(path.toStdString());
    }
    auto* editor = new SermonEditorDialog{this};
    editor->setAttribute(Qt::WA_DeleteOnClose);
    editor->set_title(QStringLiteral("Imported Sermon"));
    editor->set_files(std::move(files));
    QObject::connect(editor, &QDialog::accepted, this, [this, editor] {
        const auto imported = import_sermon_files(
            editor->files(), editor->title().toStdString(), true);
        if (imported) {
            library_dock_->add_sermon(*imported);
        }
    });
    editor->show();
}

domain::Result<domain::SermonDeck> ShekinahWindow::import_sermon_files(
    std::vector<std::filesystem::path> sources, std::string title, bool preserve_order) {
    if (!store()) {
        return domain::Result<domain::SermonDeck>::fail(
            {domain::ErrorCode::storage_failure, "Open a project before importing sermons"});
    }
    return storage::SermonDeckImporter{*store()}.import_files(
        domain::SermonId{allocate_sermon_id()}, std::move(title),
        std::move(sources), preserve_order);
}

void ShekinahWindow::create_sermon() {
    if (!store()) {
        return;
    }
    auto* editor = new SermonEditorDialog{this};
    editor->setAttribute(Qt::WA_DeleteOnClose);
    editor->set_title(QStringLiteral("New Sermon"));
    QObject::connect(editor, &QDialog::accepted, this, [this, editor] {
        const auto imported = import_sermon_files(
            editor->files(), editor->title().toStdString(), true);
        if (imported) {
            library_dock_->add_sermon(*imported);
        }
    });
    editor->show();
}

void ShekinahWindow::edit_sermon(const domain::SermonId& id) {
    if (!store() || !repository()) {
        return;
    }
    const auto existing = repository()->load_deck(id);
    if (!existing) {
        return;
    }
    std::vector<std::filesystem::path> files;
    files.reserve(existing->slides.size());
    for (const auto& slide : existing->slides) {
        const auto asset = repository()->load_asset(slide.image);
        if (!asset) {
            return;
        }
        files.push_back(store()->root() / asset->relative_path);
    }

    auto* editor = new SermonEditorDialog{this};
    editor->setAttribute(Qt::WA_DeleteOnClose);
    editor->set_title(QString::fromStdString(existing->title));
    editor->set_files(std::move(files));
    QObject::connect(editor, &QDialog::accepted, this, [this, editor, existing] {
        if (!store() || !repository()) {
            return;
        }
        const auto files = editor->files();
        std::vector<domain::SermonSlide> slides;
        slides.reserve(files.size());
        storage::AssetImporter assets{*store()};
        for (std::size_t index = 0; index != files.size(); ++index) {
            const auto imported = assets.import_file(files[index]);
            if (!imported) {
                return;
            }
            slides.push_back(domain::SermonSlide{
                domain::CueId{"slide-" + std::to_string(index)}, imported->id});
        }
        auto updated = *existing;
        updated.title = editor->title().toStdString();
        updated.slides = std::move(slides);
        const auto saved = repository()->save_deck(updated);
        if (saved) {
            refresh_library();
            presentation_.register_sermon(*saved);
            refresh_cues();
            update_program_status();
        }
    });
    editor->show();
}

domain::Status ShekinahWindow::set_slide_background(
    const domain::CueId& cue, const domain::AssetId& asset) {
    if (!repository()) {
        return domain::Status::fail(
            {domain::ErrorCode::storage_failure, "Open a project before setting a background"});
    }
    const auto selected = presentation_.selected_item();
    if (!selected.has_value() || selected->content.kind != domain::ContentKind::song) {
        return domain::Status::fail(
            {domain::ErrorCode::invalid_input, "Only selected Song cues accept editable backgrounds"});
    }
    const auto asset_record = repository()->load_asset(asset);
    if (!asset_record) {
        return domain::Status::fail(asset_record.error());
    }
    auto song = repository()->load_song(std::get<domain::SongId>(selected->content.id));
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
    const auto saved = repository()->save_song(*song);
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

void ShekinahWindow::show_inspector(const domain::CueId& cue) {
    if (!repository()) {
        return;
    }
    const auto selected = presentation_.selected_item();
    if (!selected.has_value()) {
        return;
    }
    if (selected->content.kind == domain::ContentKind::song) {
        const auto song_id = std::get<domain::SongId>(selected->content.id);
        const auto song = repository()->load_song(song_id);
        if (!song) {
            return;
        }
        const auto slide = std::find_if(song->slides.begin(), song->slides.end(), [&](const auto& candidate) {
            return candidate.id == cue;
        });
        if (slide == song->slides.end()) {
            return;
        }
        const auto fit = slide->background.has_value() ? slide->background->fit : domain::FitMode::cover;
        operator_page_->inspector()->show_slide(
            selected->content, slide->id, slide->section, slide->lines, fit);
        return;
    }

    const auto sermon_id = std::get<domain::SermonId>(selected->content.id);
    const auto sermon = repository()->load_deck(sermon_id);
    if (!sermon) {
        return;
    }
    const auto slide = std::find_if(sermon->slides.begin(), sermon->slides.end(), [&](const auto& candidate) {
        return candidate.id == cue;
    });
    if (slide != sermon->slides.end()) {
        operator_page_->inspector()->show_slide(
            selected->content, slide->id, "Sermon slide", {slide->image.value}, domain::FitMode::cover);
    }
}

void ShekinahWindow::save_inspected_slide(domain::CueId cue,
                                          std::string section,
                                          std::vector<std::string> lines,
                                          domain::FitMode fit) {
    if (!repository()) {
        return;
    }
    const auto selected = presentation_.selected_item();
    if (!selected.has_value() || selected->content.kind != domain::ContentKind::song) {
        return;
    }
    const auto song_id = std::get<domain::SongId>(selected->content.id);
    auto song = repository()->load_song(song_id);
    if (!song) {
        return;
    }
    const auto slide = std::find_if(song->slides.begin(), song->slides.end(), [&](const auto& candidate) {
        return candidate.id == cue;
    });
    if (slide == song->slides.end()) {
        return;
    }
    slide->section = std::move(section);
    slide->lines = std::move(lines);
    if (slide->background.has_value()) {
        slide->background->fit = fit;
    }
    const auto saved = repository()->save_song(*song);
    if (!saved) {
        return;
    }
    presentation_.register_song(*saved);
    refresh_library();
    refresh_cues();
    if (presentation_.program().live_cue.has_value() &&
        presentation_.program().live_cue->cue == cue) {
        update_program_status();
    }
}

void ShekinahWindow::duplicate_slide(const domain::CueId& cue) {
    if (!repository()) {
        return;
    }
    const auto selected = presentation_.selected_item();
    if (!selected.has_value()) {
        return;
    }
    if (selected->content.kind == domain::ContentKind::song) {
        const auto id = std::get<domain::SongId>(selected->content.id);
        auto song = repository()->load_song(id);
        if (!song) {
            return;
        }
        const auto position = std::find_if(song->slides.begin(), song->slides.end(), [&](const auto& slide) {
            return slide.id == cue;
        });
        if (position == song->slides.end()) {
            return;
        }
        auto duplicate = *position;
        duplicate.id = domain::CueId{song->id.value + "-copy-" + std::to_string(song->slides.size())};
        song->slides.insert(position + 1, std::move(duplicate));
        const auto saved = repository()->save_song(*song);
        if (saved) {
            presentation_.register_song(*saved);
            refresh_cues();
        }
        return;
    }

    const auto id = std::get<domain::SermonId>(selected->content.id);
    auto sermon = repository()->load_deck(id);
    if (!sermon || sermon->slides.size() == 0) {
        return;
    }
    const auto position = std::find_if(sermon->slides.begin(), sermon->slides.end(), [&](const auto& slide) {
        return slide.id == cue;
    });
    if (position == sermon->slides.end()) {
        return;
    }
    auto duplicate = *position;
    duplicate.id = domain::CueId{sermon->id.value + "-copy-" + std::to_string(sermon->slides.size())};
    sermon->slides.insert(position + 1, std::move(duplicate));
    const auto saved = repository()->save_deck(*sermon);
    if (saved) {
        presentation_.register_sermon(*saved);
        refresh_cues();
    }
}

void ShekinahWindow::delete_slide(const domain::CueId& cue) {
    if (!repository()) {
        return;
    }
    const auto selected = presentation_.selected_item();
    if (!selected.has_value()) {
        return;
    }
    if (selected->content.kind == domain::ContentKind::song) {
        const auto id = std::get<domain::SongId>(selected->content.id);
        auto song = repository()->load_song(id);
        if (!song || song->slides.size() <= 1) {
            return;
        }
        const auto position = std::find_if(song->slides.begin(), song->slides.end(), [&](const auto& slide) {
            return slide.id == cue;
        });
        if (position == song->slides.end()) {
            return;
        }
        song->slides.erase(position);
        const auto saved = repository()->save_song(*song);
        if (saved) {
            presentation_.register_song(*saved);
            if (presentation_.program().live_cue.has_value() &&
                presentation_.program().live_cue->cue == cue) {
                presentation_.clear();
            }
            refresh_cues();
            update_program_status();
        }
        return;
    }

    const auto id = std::get<domain::SermonId>(selected->content.id);
    auto sermon = repository()->load_deck(id);
    if (!sermon || sermon->slides.size() <= 1) {
        return;
    }
    const auto position = std::find_if(sermon->slides.begin(), sermon->slides.end(), [&](const auto& slide) {
        return slide.id == cue;
    });
    if (position == sermon->slides.end()) {
        return;
    }
    sermon->slides.erase(position);
    const auto saved = repository()->save_deck(*sermon);
    if (saved) {
        presentation_.register_sermon(*saved);
        if (presentation_.program().live_cue.has_value() &&
            presentation_.program().live_cue->cue == cue) {
            presentation_.clear();
        }
        refresh_cues();
        update_program_status();
    }
}

std::string ShekinahWindow::allocate_song_id() {
    for (;;) {
        const auto candidate = "song-" + std::to_string(next_content_id_++);
        if (!repository() || !repository()->load_song(domain::SongId{candidate})) {
            return candidate;
        }
    }
}

std::string ShekinahWindow::allocate_sermon_id() {
    for (;;) {
        const auto candidate = "sermon-" + std::to_string(next_content_id_++);
        if (!repository() || !repository()->load_deck(domain::SermonId{candidate})) {
            return candidate;
        }
    }
}

void ShekinahWindow::create_song() {
    if (!repository()) {
        return;
    }
    auto* editor = new ReflowEditorDialog{this};
    editor->setAttribute(Qt::WA_DeleteOnClose);
    editor->load_song(domain::SongId{allocate_song_id()});
    editor->set_title(QStringLiteral("Untitled Song"));
    QObject::connect(editor, &QDialog::accepted, this, [this, editor] {
        const auto draft = editor->draft_song();
        if (!draft || !repository()) {
            return;
        }
        auto song = *draft;
        normalize_song_cue_ids(song);
        const auto saved = repository()->save_song(song);
        if (saved) {
            library_dock_->add_song(*saved);
        }
    });
    editor->show();
}

void ShekinahWindow::edit_song(const domain::SongId& id) {
    if (!repository()) {
        return;
    }
    const auto existing = repository()->load_song(id);
    const auto reflow = repository()->load_song_reflow(id);
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
        if (!draft || !repository()) {
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
        const auto saved = repository()->save_song(song);
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
