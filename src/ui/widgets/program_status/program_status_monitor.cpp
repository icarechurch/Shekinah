#include "shekinah/ui/program_status_widget.hpp"

#include <QHBoxLayout>
#include <QVBoxLayout>

namespace shekinah::ui {

void ProgramStatusWidget::build_monitor(QVBoxLayout* layout) {
    auto* monitor_row = new QWidget{this};
    monitor_row->setObjectName(QStringLiteral("monitor-row"));
    auto* monitor_layout = new QHBoxLayout(monitor_row);
    monitor_layout->setContentsMargins(6, 6, 0, 6);
    monitor_layout->setSpacing(4);

    build_monitor_preview(monitor_row, monitor_layout);
    build_monitor_actions(monitor_row, monitor_layout);

    layout->addWidget(monitor_row);
}

}  // namespace shekinah::ui
