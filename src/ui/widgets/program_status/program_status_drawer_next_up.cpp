#include "shekinah/ui/program_status_widget.hpp"

#include "program_status_sections.hpp"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

namespace shekinah::ui {

void ProgramStatusWidget::build_next_up_card(QWidget* live_page, QVBoxLayout* live_page_layout) {
    next_up_header_button_ = new QPushButton{live_page};
    next_up_header_button_->setObjectName(QStringLiteral("next-up-header-button"));
    next_up_header_button_->setCursor(Qt::PointingHandCursor);
    next_up_header_button_->setToolTip(QStringLiteral("Advance to Next Cue (PgDn)"));
    next_up_header_button_->setIconSize(QSize{13, 13});
    QObject::connect(next_up_header_button_, &QPushButton::clicked, this, [this] {
        if (next_handler_) {
            next_handler_();
        }
    });
    live_page_layout->addWidget(
        make_section_header(QStringLiteral("NEXT UP"), live_page, next_up_header_button_));

    next_up_card_ = new QWidget{live_page};
    next_up_card_->setObjectName(QStringLiteral("next-up-card"));
    auto* next_layout = new QVBoxLayout(next_up_card_);
    next_layout->setContentsMargins(10, 8, 10, 8);
    next_layout->setSpacing(6);

    auto* next_meta = new QHBoxLayout;
    next_section_badge_ = new QLabel{QStringLiteral("STANDBY"), next_up_card_};
    next_section_badge_->setObjectName(QStringLiteral("next-section-badge"));
    next_meta->addWidget(next_section_badge_);
    next_meta->addStretch(1);
    next_layout->addLayout(next_meta);

    next_summary_label_ = new QLabel{
        QStringLiteral("Select a Song or Sermon in the Service Order to cue slides."),
        next_up_card_};
    next_summary_label_->setObjectName(QStringLiteral("next-summary-label"));
    next_summary_label_->setMinimumWidth(0);
    next_summary_label_->setWordWrap(true);
    next_layout->addWidget(next_summary_label_);

    next_up_action_button_ = new QPushButton{QStringLiteral("Show Audience Window"), next_up_card_};
    next_up_action_button_->setObjectName(QStringLiteral("next-up-action-button"));
    next_up_action_button_->setCursor(Qt::PointingHandCursor);
    QObject::connect(next_up_action_button_, &QPushButton::clicked, this, [this] {
        if (current_program_.mode == domain::ProgramMode::cue && !live_details_.next_summary.isEmpty()) {
            if (next_handler_) {
                next_handler_();
            }
        } else if (show_movable_handler_) {
            show_movable_handler_();
        }
    });
    next_layout->addWidget(next_up_action_button_);
    next_layout->addStretch(1);
    live_page_layout->addWidget(next_up_card_, 1);
}

}  // namespace shekinah::ui
