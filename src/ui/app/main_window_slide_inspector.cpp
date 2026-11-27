#include "shekinah/ui/main_window.hpp"

#include "slide_inspection_coordinator.hpp"
#include "shekinah/ui/operator_page.hpp"

namespace shekinah::ui {

void ShekinahWindow::show_inspector(const domain::CueId& cue) {
    const auto selected = presentation_.selected_item();
    if (!selected.has_value()) {
        return;
    }
    const auto inspection = slide_inspection_coordinator_->inspect(selected->content, cue);
    if (!inspection.has_value()) {
        return;
    }
    operator_page_->inspector()->show_slide(selected->content,
                                            inspection->cue,
                                            inspection->section,
                                            inspection->lines,
                                            inspection->fit);
}

void ShekinahWindow::save_inspected_slide(domain::CueId cue,
                                          std::string section,
                                          std::vector<std::string> lines,
                                          domain::FitMode fit) {
    const auto selected = presentation_.selected_item();
    if (!selected.has_value() || selected->content.kind != domain::ContentKind::song) {
        return;
    }
    if (!slide_inspection_coordinator_->save_song_slide(
            selected->content, cue, std::move(section), std::move(lines), fit)) {
        return;
    }
    refresh_library();
    refresh_cues();
    if (presentation_.program().live_cue.has_value() &&
        presentation_.program().live_cue->cue == cue) {
        update_program_status();
    }
}

}  // namespace shekinah::ui
