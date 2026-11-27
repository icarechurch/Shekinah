#include "shekinah/ui/main_window.hpp"

#include "cue_presentation_coordinator.hpp"
#include "main_window_cue_views.hpp"
#include "shekinah/ui/cue_grid_widget.hpp"
#include "shekinah/ui/operator_page.hpp"
#include "shekinah/ui/program_status_widget.hpp"

namespace shekinah::ui {

void ShekinahWindow::refresh_cues() {
    const auto selected = presentation_.selected_item();
    if (!selected.has_value() || !cue_presentation_coordinator_->ready()) {
        operator_page_->cue_grid()->set_cues({});
        operator_page_->program_status()->set_item_cues({}, {});
        operator_page_->set_active_presentation_title({});
        return;
    }

    const auto presented = cue_presentation_coordinator_->load(selected->content);
    if (!presented) {
        operator_page_->cue_grid()->set_cues({});
        operator_page_->program_status()->set_item_cues({}, {});
        return;
    }

    auto views = main_window_internal::make_cue_views(*presented);
    operator_page_->set_active_presentation_title(views.title);
    operator_page_->cue_grid()->set_cues(std::move(views.cues));
    operator_page_->program_status()->set_item_cues(
        std::move(views.title), std::move(views.quick_cues));
}

}  // namespace shekinah::ui
