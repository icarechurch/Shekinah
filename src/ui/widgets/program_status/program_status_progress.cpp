#include "shekinah/ui/program_status_widget.hpp"

#include <QProgressBar>
#include <QVBoxLayout>

namespace shekinah::ui {

void ProgramStatusWidget::build_global_progress(QVBoxLayout* layout) {
    slide_progress_ = new QProgressBar{this};
    slide_progress_->setObjectName(QStringLiteral("slide-progress"));
    slide_progress_->setTextVisible(false);
    slide_progress_->setFixedHeight(3);
    slide_progress_->setRange(0, 100);
    slide_progress_->setValue(0);
    layout->addWidget(slide_progress_);
}

}  // namespace shekinah::ui
