#include "shekinah/ui/main_window.hpp"

#include "shekinah/media/audience_output.hpp"
#include "shekinah/ui/operator_page.hpp"
#include "shekinah/ui/program_status_widget.hpp"
#include "shekinah/ui/support/theme.hpp"

#include <QActionGroup>
#include <QFileDialog>
#include <QKeySequence>
#include <QMenu>
#include <QMenuBar>

namespace shekinah::ui {

void ShekinahWindow::configure_window_actions() {
    auto* window_menu = menuBar()->addMenu(QStringLiteral("Window"));
    auto* audience_action = window_menu->addAction(QStringLiteral("Show Audience Output"));
    audience_action->setObjectName(QStringLiteral("show-audience-output"));
    QObject::connect(audience_action, &QAction::triggered, this, [this] {
        audience_output()->show_movable();
        operator_page_->program_status()->set_output_status(
            QStringLiteral("Output: Window"), QStringLiteral("Audience window"), true);
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
}

}  // namespace shekinah::ui
