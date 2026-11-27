#include "shekinah/ui/program_status_widget.hpp"

#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

namespace shekinah::ui {

void ProgramStatusWidget::build_header(QVBoxLayout* layout) {
    auto* header_bar = new QWidget{this};
    header_bar->setObjectName(QStringLiteral("program-header-bar"));
    header_bar->setFixedHeight(26);
    auto* header_layout = new QHBoxLayout(header_bar);
    header_layout->setContentsMargins(10, 0, 8, 0);
    header_layout->setSpacing(6);

    auto* heading = new QLabel{QStringLiteral("PROGRAM STATUS"), header_bar};
    heading->setObjectName(QStringLiteral("program-status-title"));
    header_layout->addWidget(heading);
    header_layout->addStretch(1);

    mode_label_ = new QLabel{QStringLiteral("CLEAR"), header_bar};
    mode_label_->setObjectName(QStringLiteral("program-mode"));
    mode_label_->setProperty("role", QStringLiteral("status"));
    mode_label_->setAlignment(Qt::AlignCenter);
    header_layout->addWidget(mode_label_);
    layout->addWidget(header_bar);
}

}  // namespace shekinah::ui
