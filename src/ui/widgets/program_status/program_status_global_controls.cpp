#include "shekinah/ui/program_status_widget.hpp"

#include <QVBoxLayout>

namespace shekinah::ui {

void ProgramStatusWidget::build_global_controls(QVBoxLayout* layout) {
    build_global_progress(layout);
    build_global_actions(layout);
}

}  // namespace shekinah::ui
