#include "shekinah/ui/main_window.hpp"

#include <QAction>
#include <QKeySequence>
#include <QMenu>
#include <QMenuBar>

namespace shekinah::ui {

void ShekinahWindow::configure_presentation_menu() {
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

}  // namespace shekinah::ui
