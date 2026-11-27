#include "shekinah/ui/main_window.hpp"

#include "slide_mutation_coordinator.hpp"

namespace shekinah::ui {

void ShekinahWindow::delete_slide(const domain::CueId& cue) {
    const auto selected = presentation_.selected_item();
    if (!selected.has_value()) {
        return;
    }
    if (!slide_mutation_coordinator_->erase(selected->content, cue)) {
        return;
    }
    if (presentation_.program().live_cue.has_value() &&
        presentation_.program().live_cue->cue == cue) {
        presentation_.clear();
    }
    refresh_cues();
    update_program_status();
}

}  // namespace shekinah::ui
