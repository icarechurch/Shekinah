#include "shekinah/ui/program_status_widget.hpp"

#include "program_status_sections.hpp"

#include "shekinah/ui/support/elided_label.hpp"

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QSizePolicy>
#include <QStackedWidget>
#include <QVBoxLayout>

namespace shekinah::ui {

void ProgramStatusWidget::build_live_drawer_page() {
    auto* live_scroll = new QScrollArea{drawer_stack_};
    live_scroll->setObjectName(QStringLiteral("drawer-page-live-scroll"));
    live_scroll->setMinimumHeight(48);
    live_scroll->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Ignored);
    live_scroll->setFrameShape(QFrame::NoFrame);
    live_scroll->setWidgetResizable(true);
    live_scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    live_scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    auto* live_page = new QWidget{live_scroll};
    live_page->setObjectName(QStringLiteral("drawer-page-live"));
    auto* live_page_layout = new QVBoxLayout(live_page);
    live_page_layout->setContentsMargins(0, 0, 0, 0);
    live_page_layout->setSpacing(0);

    live_edit_button_ = new QPushButton{QStringLiteral("Inspect"), live_page};
    live_edit_button_->setObjectName(QStringLiteral("live-inspect-button"));
    live_edit_button_->setProperty("linkAction", true);
    live_edit_button_->setCursor(Qt::PointingHandCursor);
    QObject::connect(live_edit_button_, &QPushButton::clicked, this, [this] {
        if (inspect_live_handler_) {
            inspect_live_handler_();
        }
    });
    live_page_layout->addWidget(
        make_section_header(QStringLiteral("LIVE SLIDE"), live_page, live_edit_button_));

    auto* live_card = new QWidget{live_page};
    live_card->setObjectName(QStringLiteral("live-card"));
    auto* live_card_layout = new QVBoxLayout(live_card);
    live_card_layout->setContentsMargins(10, 8, 10, 8);
    live_card_layout->setSpacing(6);

    auto* meta_row = new QHBoxLayout;
    meta_row->setSpacing(6);
    live_section_badge_ = new QLabel{QStringLiteral("IDLE"), live_card};
    live_section_badge_->setObjectName(QStringLiteral("live-section-badge"));
    meta_row->addWidget(live_section_badge_);

    live_item_title_label_ = new ElidedLabel{QStringLiteral("Ready for service"), live_card};
    live_item_title_label_->setObjectName(QStringLiteral("live-item-title"));
    live_item_title_label_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    meta_row->addWidget(live_item_title_label_, 1);

    live_slide_counter_label_ = new QLabel{QStringLiteral("--"), live_card};
    live_slide_counter_label_->setObjectName(QStringLiteral("live-slide-counter"));
    meta_row->addWidget(live_slide_counter_label_);
    live_card_layout->addLayout(meta_row);

    live_lyrics_label_ = new QLabel{
        QStringLiteral("Click any cue in the grid to project it live to the Audience Output."),
        live_card};
    live_lyrics_label_->setObjectName(QStringLiteral("live-lyrics-preview"));
    live_lyrics_label_->setMinimumWidth(0);
    live_lyrics_label_->setWordWrap(true);
    live_card_layout->addWidget(live_lyrics_label_);

    auto* bg_row = new QHBoxLayout;
    bg_row->setSpacing(6);
    live_asset_label_ = new ElidedLabel{QStringLiteral("Background: None"), live_card};
    live_asset_label_->setObjectName(QStringLiteral("live-asset-label"));
    live_asset_label_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    bg_row->addWidget(live_asset_label_, 1);

    live_bg_button_ = new QPushButton{QStringLiteral("Set Bg"), live_card};
    live_bg_button_->setObjectName(QStringLiteral("live-set-bg-button"));
    live_bg_button_->setCursor(Qt::PointingHandCursor);
    QObject::connect(live_bg_button_, &QPushButton::clicked, this, [this] {
        if (choose_background_handler_) {
            choose_background_handler_();
        }
    });
    bg_row->addWidget(live_bg_button_);
    live_card_layout->addLayout(bg_row);
    live_page_layout->addWidget(live_card);

    build_next_up_card(live_page, live_page_layout);
    live_scroll->setWidget(live_page);
    drawer_stack_->addWidget(live_scroll);
}

}  // namespace shekinah::ui
