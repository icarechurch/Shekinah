#include "shekinah/ui/operator_page.hpp"

#include "shekinah/ui/program_status_widget.hpp"

#include <utility>

namespace shekinah::ui {

void OperatorPage::set_previous_handler(std::function<void()> handler) {
    program_status_->set_previous_handler(std::move(handler));
}

void OperatorPage::set_next_handler(std::function<void()> handler) {
    program_status_->set_next_handler(std::move(handler));
}

void OperatorPage::set_clear_handler(std::function<void()> handler) {
    program_status_->set_clear_handler(std::move(handler));
}

void OperatorPage::set_black_handler(std::function<void()> handler) {
    program_status_->set_black_handler(std::move(handler));
}

void OperatorPage::set_show_screen_handler(std::function<void(QScreen*)> handler) {
    program_status_->set_show_screen_handler(std::move(handler));
}

void OperatorPage::set_show_movable_handler(std::function<void()> handler) {
    program_status_->set_show_movable_handler(std::move(handler));
}

void OperatorPage::set_hide_output_handler(std::function<void()> handler) {
    program_status_->set_hide_output_handler(std::move(handler));
}

void OperatorPage::set_inspect_live_handler(std::function<void()> handler) {
    program_status_->set_inspect_live_handler(std::move(handler));
}

void OperatorPage::set_choose_background_handler(std::function<void()> handler) {
    program_status_->set_choose_background_handler(std::move(handler));
}

}  // namespace shekinah::ui
