#include "shekinah/ui/main_window.hpp"

#include "slide_mutation_coordinator.hpp"

namespace shekinah::ui {

void ShekinahWindow::duplicate_slide(const domain::CueId& cue) {
    const auto selected = presentation_.selected_item();
    if (!selected.has_value()) {
        return;
    }
    if (!slide_mutation_coordinator_->duplicate(selected->content, cue)) {
        return;
    }
    refresh_cues();
}

}  // namespace shekinah::ui
