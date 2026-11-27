#include "shekinah/ui/program_status_widget.hpp"

#include "program_status_preview.hpp"

#include <QLabel>

#include <utility>

namespace shekinah::ui {

void ProgramStatusWidget::set_program_title(QString title) {
    const auto trimmed = title.trimmed();
    set_slide_preview(trimmed.isEmpty() ? QStringLiteral("No cue live") : trimmed);
}

void ProgramStatusWidget::set_slide_preview(QString text,
                                            QString background_path,
                                            domain::FitMode background_fit) {
    auto* preview = static_cast<ProgramSlidePreview*>(slide_preview_);
    preview->set_content(std::move(text), std::move(background_path), background_fit);
}

void ProgramStatusWidget::set_program(const domain::ProgramState& program) {
    current_program_ = program;
    auto* preview = static_cast<ProgramSlidePreview*>(slide_preview_);
    preview->set_mode(program.mode);
    mode_label_->setText(mode_text(program.mode));
    cue_label_->setText(program.live_cue.has_value()
                            ? QString::fromStdString(program.live_cue->cue.value)
                            : QStringLiteral("No cue live"));
    if (!program.live_cue.has_value()) {
        set_slide_preview({});
        live_details_ = {};
        set_live_details(live_details_);
    }
    update_status_chrome();
}

}  // namespace shekinah::ui
