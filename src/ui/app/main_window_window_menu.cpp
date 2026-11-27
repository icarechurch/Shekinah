#include "shekinah/ui/main_window.hpp"

#include "shekinah/media/audience_output.hpp"
#include "shekinah/ui/operator_page.hpp"
#include "shekinah/ui/program_status_widget.hpp"

#include <QAction>
#include <QMenu>
#include <QMenuBar>

namespace shekinah::ui {

void ShekinahWindow::configure_window_menu() {
    auto* window_menu = menuBar()->addMenu(QStringLiteral("Window"));
    auto* audience_action = window_menu->addAction(QStringLiteral("Show Audience Output"));
    audience_action->setObjectName(QStringLiteral("show-audience-output"));
    QObject::connect(audience_action, &QAction::triggered, this, [this] {
        audience_output()->show_movable();
        operator_page_->program_status()->set_output_status(
            QStringLiteral("Output: Window"), QStringLiteral("Audience window"), true);
    });
}

}  // namespace shekinah::ui
