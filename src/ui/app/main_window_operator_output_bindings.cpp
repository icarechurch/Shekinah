#include "shekinah/ui/main_window.hpp"

#include "shekinah/media/audience_output.hpp"
#include "shekinah/ui/operator_page.hpp"
#include "shekinah/ui/program_status_widget.hpp"

#include <QScreen>

namespace shekinah::ui {

void ShekinahWindow::configure_operator_output_handlers() {
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
}

}  // namespace shekinah::ui
