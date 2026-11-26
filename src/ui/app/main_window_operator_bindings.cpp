#include "shekinah/ui/main_window.hpp"

#include "shekinah/media/audience_output.hpp"
#include "shekinah/storage/content_repository.hpp"
#include "shekinah/ui/operator_page.hpp"
#include "shekinah/ui/program_status_widget.hpp"

#include <QScreen>

namespace shekinah::ui {

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

}  // namespace shekinah::ui
