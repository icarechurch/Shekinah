#include "shekinah/ui/main_window.hpp"

#include <QAction>
#include <QFileDialog>
#include <QKeySequence>
#include <QMenu>
#include <QMenuBar>

namespace shekinah::ui {

void ShekinahWindow::configure_file_menu() {
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
}

}  // namespace shekinah::ui
