#include "shekinah/ui/program_status_widget.hpp"

#include <QSizePolicy>
#include <QStackedWidget>
#include <QVBoxLayout>

namespace shekinah::ui {

void ProgramStatusWidget::build_drawer_pages(QVBoxLayout* layout) {
    drawer_stack_ = new QStackedWidget{this};
    drawer_stack_->setObjectName(QStringLiteral("drawer-stack"));
    drawer_stack_->setMinimumHeight(60);
    drawer_stack_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Ignored);

    build_live_drawer_page();
    build_screens_drawer_page();
    build_cues_drawer_page();

    layout->addWidget(drawer_stack_, 1);
}

}  // namespace shekinah::ui
