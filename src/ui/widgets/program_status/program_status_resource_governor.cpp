#include "shekinah/ui/program_status_widget.hpp"

#include "program_status_sections.hpp"

#include <QLabel>
#include <QProgressBar>
#include <QVBoxLayout>

namespace shekinah::ui {

void ProgramStatusWidget::build_resource_governor(QWidget* screens_page,
                                                   QVBoxLayout* screens_layout) {
    screens_layout->addWidget(make_section_header(QStringLiteral("RESOURCE GOVERNOR"), screens_page));
    auto* mem_panel = new QWidget{screens_page};
    mem_panel->setObjectName(QStringLiteral("memory-panel"));
    auto* mem_layout = new QVBoxLayout(mem_panel);
    mem_layout->setContentsMargins(10, 8, 10, 10);
    mem_layout->setSpacing(6);

    memory_status_label_ = new QLabel{
        QStringLiteral("Memory: -- / 270 MB \xC2\xB7 Live Video Admitted"), mem_panel};
    memory_status_label_->setObjectName(QStringLiteral("memory-status-label"));
    memory_status_label_->setMinimumWidth(0);
    memory_status_label_->setWordWrap(true);
    mem_layout->addWidget(memory_status_label_);

    memory_bar_ = new QProgressBar{mem_panel};
    memory_bar_->setObjectName(QStringLiteral("memory-usage-bar"));
    memory_bar_->setTextVisible(false);
    memory_bar_->setFixedHeight(6);
    memory_bar_->setRange(0, 300);
    memory_bar_->setValue(0);
    mem_layout->addWidget(memory_bar_);
    screens_layout->addWidget(mem_panel);
}

}  // namespace shekinah::ui
