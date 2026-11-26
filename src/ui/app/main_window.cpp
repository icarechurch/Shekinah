#include "shekinah/ui/main_window.hpp"

#include "shekinah/application/cue_presenter.hpp"
#include "shekinah/application/live_slide_presenter.hpp"
#include "shekinah/application/resource_governor.hpp"
#include "shekinah/application/service_order_presenter.hpp"
#include "shekinah/application/slide_insertion.hpp"
#include "shekinah/application/song_backgrounds.hpp"
#include "shekinah/application/song_authoring.hpp"
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

}  // namespace shekinah::ui
