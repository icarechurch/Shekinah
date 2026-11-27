#include "shekinah/ui/main_window.hpp"

#include "shekinah/media/audience_output.hpp"
#include "shekinah/ui/cue_grid_widget.hpp"
#include "shekinah/ui/operator_page.hpp"
#include "shekinah/ui/program_status_widget.hpp"

#include <optional>

namespace shekinah::ui {

void ShekinahWindow::update_program_status() {
    const auto program = presentation_.program();
    std::optional<domain::CueId> selected_live_cue;
    const auto selected_item = presentation_.selected_item();
    if (program.live_cue.has_value() && selected_item.has_value() &&
        program.live_cue->content == selected_item->content) {
        selected_live_cue = program.live_cue->cue;
    }
    operator_page_->set_service_order_live_content(
        program.mode == domain::ProgramMode::cue && program.live_cue.has_value()
            ? std::optional<domain::ContentRef>{program.live_cue->content}
            : std::nullopt);
    operator_page_->cue_grid()->set_live_cue(std::move(selected_live_cue));
    operator_page_->program_status()->set_program(program);
    if (audience_output_) {
        audience_output_->present(presentation_.program());
    }
    present_live_media();
}

}  // namespace shekinah::ui
