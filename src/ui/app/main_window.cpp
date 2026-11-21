#include "shekinah/ui/main_window.hpp"

#include "shekinah/application/resource_governor.hpp"
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

#include <QFileDialog>
#include <QInputDialog>
#include <QKeySequence>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
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
    library_dock_->set_new_sermon_handler([this] { create_sermon(); });
    library_dock_->set_edit_sermon_handler([this](domain::SermonId id) { edit_sermon(id); });
    library_dock_->set_edit_song_handler([this](domain::SongId id) { edit_song(id); });
    operator_page_->set_authoring_handlers(
        [this] { create_song(); },
        [this] { import_song_text(); },
        [this] { import_sermon_deck(); },
        [this] { import_asset(); });
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
    operator_page_->set_cue_asset_drop_handler(
        [this](domain::CueId cue, domain::AssetId asset) {
            apply_asset_to_slide(std::move(cue), std::move(asset));
        });
    operator_page_->set_inspector_save_handler(
        [this](domain::CueId cue, std::string section, std::vector<std::string> lines, domain::FitMode fit) {
            save_inspected_slide(std::move(cue), std::move(section), std::move(lines), fit);
        });
    library_dock_->setMinimumHeight(190);
    library_dock_->show();
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
        repository_.reset();
        store_.reset();
        project_root_.clear();
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

    library_dock_->set_asset_root(project_root_);
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
        std::optional<domain::BackgroundRef> previous_background;
        for (std::size_t index = 0; index != song->slides.size(); ++index) {
            const auto& slide = song->slides[index];
            std::ostringstream summary;
            for (std::size_t index = 0; index != slide.lines.size(); ++index) {
                if (index != 0) {
                    summary << '\n';
                }
                summary << slide.lines[index];
            }

            QString background_path;
            domain::FitMode background_fit = domain::FitMode::cover;
            const auto background = slide.background.has_value()
                                      ? slide.background
                                      : song->default_background;
            const auto starts_asset_range = background.has_value() &&
                                            (!previous_background.has_value() ||
                                             previous_background != background);
            if (starts_asset_range && store_) {
                const auto asset = repository_->load_asset(background->asset);
                if (asset) {
                    background_path = QString::fromStdString(
                        (store_->root() / asset->relative_path).string());
                    background_fit = background->fit;
                }
            }
            previous_background = background;
            cues.push_back(CueView{
                slide.id,
                QString::fromStdString(slide.section),
                QString::fromStdString(summary.str()),
                static_cast<int>(index + 1),
                std::move(background_path),
                background_fit,
                QString::fromStdString(slide.section),
                true,
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
            QString background_path;
            if (store_) {
                const auto asset = repository_->load_asset(sermon->slides[index].image);
                if (asset) {
                    background_path = QString::fromStdString(
                        (store_->root() / asset->relative_path).string());
                }
            }
            cues.push_back(CueView{
                sermon->slides[index].id,
                {},
                {},
                static_cast<int>(index + 1),
                std::move(background_path),
                domain::FitMode::cover,
                QStringLiteral("Slide %1").arg(static_cast<qulonglong>(index + 1)),
                false,
            });
        }
    }
    operator_page_->cue_grid()->set_cues(std::move(cues));
}

void ShekinahWindow::update_program_status() {
    const auto program = presentation_.program();
    std::optional<domain::CueId> selected_live_cue;
    const auto selected_item = presentation_.selected_item();
    if (program.live_cue.has_value() && selected_item.has_value() &&
        program.live_cue->content == selected_item->content) {
        selected_live_cue = program.live_cue->cue;
    }
    operator_page_->cue_grid()->set_live_cue(std::move(selected_live_cue));
    operator_page_->program_status()->set_program(program);
    if (audience_output_) {
        audience_output_->present(presentation_.program());
    }
    present_live_media();
}

void ShekinahWindow::present_live_media() {
    if (!repository_) {
        operator_page_->program_status()->set_slide_preview({});
        return;
    }
    const auto& program = presentation_.program();
    if (program.mode != domain::ProgramMode::cue || !program.live_cue.has_value()) {
        operator_page_->program_status()->set_slide_preview({});
        if (audience_output_) {
            audience_output_->set_live_text({});
            audience_output_->clear_slide_image();
            audience_output_->clear_loop_video();
        }
        return;
    }

    std::optional<domain::AssetId> asset_id;
    QString preview_text;
    domain::FitMode preview_fit = domain::FitMode::cover;
    const auto& cue = *program.live_cue;
    if (cue.content.kind == domain::ContentKind::song) {
        const auto song = repository_->load_song(std::get<domain::SongId>(cue.content.id));
        if (song) {
            const auto slide = std::find_if(song->slides.begin(), song->slides.end(), [&](const auto& candidate) {
                return candidate.id == cue.cue;
            });
            if (slide != song->slides.end()) {
                std::ostringstream lyric_text;
                for (std::size_t index = 0; index != slide->lines.size(); ++index) {
                    if (index != 0) {
                        lyric_text << '\n';
                    }
                    lyric_text << slide->lines[index];
                }
                preview_text = QString::fromStdString(lyric_text.str());
            }
        }
        const auto background = presentation_.resolve_background(cue.content, cue.cue);
        if (background && background->has_value()) {
            asset_id = background->value().asset;
            preview_fit = background->value().fit;
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

    if (audience_output_) {
        audience_output_->set_live_text(preview_text);
    }

    if (!asset_id.has_value()) {
        operator_page_->program_status()->set_slide_preview(std::move(preview_text));
        if (audience_output_) {
            audience_output_->clear_slide_image();
            audience_output_->clear_loop_video();
        }
        return;
    }
    const auto asset = repository_->load_asset(*asset_id);
    if (!asset) {
        operator_page_->program_status()->set_slide_preview(std::move(preview_text));
        return;
    }
    const auto path = store_->root() / asset->relative_path;
    if (asset->kind == domain::AssetKind::image) {
        operator_page_->program_status()->set_slide_preview(
            std::move(preview_text), QString::fromStdString(path.string()), preview_fit);
        if (audience_output_) {
            audience_output_->clear_loop_video();
            audience_output_->load_slide_image(path, preview_fit);
        }
    } else {
        operator_page_->program_status()->set_slide_preview(std::move(preview_text));
        if (!audience_output_) {
            return;
        }
        if (!live_media_admitted()) {
            return;
        }
        audience_output_->clear_slide_image();
        audience_output_->load_loop_video(path);
    }
}

QString ShekinahWindow::asset_name_for_cue(const domain::CueId& cue) const {
    if (!repository_) {
        return {};
    }
    const auto selected = presentation_.selected_item();
    if (!selected.has_value() || selected->content.kind != domain::ContentKind::song) {
        return {};
    }
    const auto background = presentation_.resolve_background(selected->content, cue);
    if (!background || !background->has_value()) {
        return {};
    }
    const auto asset = repository_->load_asset(background->value().asset);
    if (!asset) {
        return {};
    }
    const auto filename = std::filesystem::path{asset->relative_path}.filename().string();
    return QString::fromStdString(filename.empty() ? asset->relative_path : filename);
}

void ShekinahWindow::remove_asset_from_song(const domain::CueId& cue) {
    if (!repository_) {
        return;
    }
    const auto selected = presentation_.selected_item();
    if (!selected.has_value() || selected->content.kind != domain::ContentKind::song) {
        return;
    }
    const auto background = presentation_.resolve_background(selected->content, cue);
    if (!background || !background->has_value()) {
        return;
    }
    auto song = repository_->load_song(std::get<domain::SongId>(selected->content.id));
    if (!song) {
        return;
    }
    application::remove_asset_from_song(*song, background->value().asset);
    const auto saved = repository_->save_song(*song);
    if (!saved) {
        return;
    }
    presentation_.register_song(*saved);
    refresh_cues();
    update_program_status();
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
        this, QStringLiteral("Import Asset"), {},
        QStringLiteral(
            "Media (*.png *.jpg *.jpeg *.webp *.bmp *.gif *.tif *.tiff *.mp4);;All files (*)"));
    if (path.isEmpty()) {
        return;
    }
    const auto imported = import_asset_file(path.toStdString());
    if (imported) {
        refresh_library();
        library_dock_->show_assets();
    } else {
        QMessageBox::warning(
            this, QStringLiteral("Asset import failed"), QString::fromStdString(imported.error().message));
    }
}

void ShekinahWindow::apply_asset_to_song(domain::ContentRef content, domain::AssetId asset) {
    if (!repository_ || content.kind != domain::ContentKind::song) {
        return;
    }
    const auto asset_record = repository_->load_asset(asset);
    if (!asset_record) {
        return;
    }
    auto song = repository_->load_song(std::get<domain::SongId>(content.id));
    if (!song) {
        return;
    }
    application::set_default_background(
        *song, domain::BackgroundRef{asset_record->id, domain::FitMode::cover});
    const auto saved = repository_->save_song(*song);
    if (!saved) {
        return;
    }
    presentation_.register_song(*saved);
    refresh_cues();
    update_program_status();
}

void ShekinahWindow::apply_asset_to_slide(domain::CueId cue, domain::AssetId asset) {
    if (!repository_) {
        return;
    }
    const auto selected = presentation_.selected_item();
    if (!selected.has_value() || selected->content.kind != domain::ContentKind::song) {
        return;
    }
    const auto asset_record = repository_->load_asset(asset);
    if (!asset_record) {
        return;
    }
    auto song = repository_->load_song(std::get<domain::SongId>(selected->content.id));
    if (!song) {
        return;
    }
    const auto changed = application::apply_background_from_slide(
        *song, cue, domain::BackgroundRef{asset_record->id, domain::FitMode::cover});
    if (!changed) {
        return;
    }
    const auto saved = repository_->save_song(*song);
    if (!saved) {
        return;
    }
    presentation_.register_song(*saved);
    refresh_cues();
    update_program_status();
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
    if (!store_) {
        return domain::Result<domain::SermonDeck>::fail(
            {domain::ErrorCode::storage_failure, "Open a project before importing sermons"});
    }
    return storage::SermonDeckImporter{*store_}.import_pngs(
        domain::SermonId{allocate_sermon_id()}, std::move(title),
        std::move(sources), preserve_order);
}

void ShekinahWindow::create_sermon() {
    if (!store_) {
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
    if (!store_ || !repository_) {
        return;
    }
    const auto existing = repository_->load_deck(id);
    if (!existing) {
        return;
    }
    std::vector<std::filesystem::path> files;
    files.reserve(existing->slides.size());
    for (const auto& slide : existing->slides) {
        const auto asset = repository_->load_asset(slide.image);
        if (!asset) {
            return;
        }
        files.push_back(store_->root() / asset->relative_path);
    }

    auto* editor = new SermonEditorDialog{this};
    editor->setAttribute(Qt::WA_DeleteOnClose);
    editor->set_title(QString::fromStdString(existing->title));
    editor->set_files(std::move(files));
    QObject::connect(editor, &QDialog::accepted, this, [this, editor, existing] {
        if (!store_ || !repository_) {
            return;
        }
        const auto files = editor->files();
        std::vector<domain::SermonSlide> slides;
        slides.reserve(files.size());
        storage::AssetImporter assets{*store_};
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
        const auto saved = repository_->save_deck(updated);
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

void ShekinahWindow::show_inspector(const domain::CueId& cue) {
    if (!repository_) {
        return;
    }
    const auto selected = presentation_.selected_item();
    if (!selected.has_value()) {
        return;
    }
    if (selected->content.kind == domain::ContentKind::song) {
        const auto song_id = std::get<domain::SongId>(selected->content.id);
        const auto song = repository_->load_song(song_id);
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
    const auto sermon = repository_->load_deck(sermon_id);
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
    if (!repository_) {
        return;
    }
    const auto selected = presentation_.selected_item();
    if (!selected.has_value() || selected->content.kind != domain::ContentKind::song) {
        return;
    }
    const auto song_id = std::get<domain::SongId>(selected->content.id);
    auto song = repository_->load_song(song_id);
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
    const auto saved = repository_->save_song(*song);
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
    if (!repository_) {
        return;
    }
    const auto selected = presentation_.selected_item();
    if (!selected.has_value()) {
        return;
    }
    if (selected->content.kind == domain::ContentKind::song) {
        const auto id = std::get<domain::SongId>(selected->content.id);
        auto song = repository_->load_song(id);
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
        const auto saved = repository_->save_song(*song);
        if (saved) {
            presentation_.register_song(*saved);
            refresh_cues();
        }
        return;
    }

    const auto id = std::get<domain::SermonId>(selected->content.id);
    auto sermon = repository_->load_deck(id);
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
    const auto saved = repository_->save_deck(*sermon);
    if (saved) {
        presentation_.register_sermon(*saved);
        refresh_cues();
    }
}

void ShekinahWindow::delete_slide(const domain::CueId& cue) {
    if (!repository_) {
        return;
    }
    const auto selected = presentation_.selected_item();
    if (!selected.has_value()) {
        return;
    }
    if (selected->content.kind == domain::ContentKind::song) {
        const auto id = std::get<domain::SongId>(selected->content.id);
        auto song = repository_->load_song(id);
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
        const auto saved = repository_->save_song(*song);
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
    auto sermon = repository_->load_deck(id);
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
    const auto saved = repository_->save_deck(*sermon);
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
