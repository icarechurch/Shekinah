#include "shekinah/ui/program_status_widget.hpp"

#include "program_status_components.hpp"

namespace shekinah::ui {

void ProgramStatusWidget::build_drawer_pages(QVBoxLayout* layout) {
        // 7. Stacked Drawer Pages
        drawer_stack_ = new QStackedWidget{this};
        drawer_stack_->setObjectName(QStringLiteral("drawer-stack"));
        drawer_stack_->setMinimumHeight(60);
        drawer_stack_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Ignored);

        // --- Page 0: Live & Next (scrollable when window height is constrained) ---
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
        live_scroll->setWidget(live_page);
        drawer_stack_->addWidget(live_scroll);

        // --- Page 1: Screens & Diagnostics (scrollable when height is constrained) ---
        auto* screens_scroll = new QScrollArea{drawer_stack_};
        screens_scroll->setObjectName(QStringLiteral("drawer-page-screens-scroll"));
        screens_scroll->setMinimumHeight(48);
        screens_scroll->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Ignored);
        screens_scroll->setFrameShape(QFrame::NoFrame);
        screens_scroll->setWidgetResizable(true);
        screens_scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        screens_scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

        auto* screens_page = new QWidget{screens_scroll};
        screens_page->setObjectName(QStringLiteral("drawer-page-screens"));
        auto* screens_layout = new QVBoxLayout(screens_page);
        screens_layout->setContentsMargins(0, 0, 0, 0);
        screens_layout->setSpacing(0);

        auto* refresh_screens_btn = new QPushButton{QStringLiteral("Refresh"), screens_page};
        refresh_screens_btn->setProperty("linkAction", true);
        refresh_screens_btn->setCursor(Qt::PointingHandCursor);
        QObject::connect(refresh_screens_btn, &QPushButton::clicked, this, [this] {
            refresh_screens_list();
        });
        screens_layout->addWidget(
            make_section_header(QStringLiteral("AUDIENCE DISPLAYS"), screens_page, refresh_screens_btn));

        screens_list_ = new QListWidget{screens_page};
        screens_list_->setObjectName(QStringLiteral("screens-list"));
        screens_list_->setMinimumHeight(48);
        screens_list_->setSelectionMode(QAbstractItemView::SingleSelection);
        QObject::connect(screens_list_, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem* item) {
            if (item == nullptr || !show_screen_handler_) {
                return;
            }
            const int idx = item->data(Qt::UserRole).toInt();
            const auto screens = QGuiApplication::screens();
            if (idx >= 0 && idx < screens.size()) {
                show_screen_handler_(screens.at(idx));
            }
        });
        screens_layout->addWidget(screens_list_, 1);

        auto* screen_actions = new QWidget{screens_page};
        screen_actions->setObjectName(QStringLiteral("screen-actions-bar"));
        auto* screen_actions_layout = new QHBoxLayout(screen_actions);
        screen_actions_layout->setContentsMargins(8, 6, 8, 6);
        screen_actions_layout->setSpacing(6);

        const auto add_screen_btn = [&](const QString& label, const QString& name, auto handler) {
            auto* btn = new QPushButton{label, screen_actions};
            btn->setObjectName(name);
            btn->setProperty("screenAction", true);
            btn->setCursor(Qt::PointingHandCursor);
            QObject::connect(btn, &QPushButton::clicked, this, handler);
            screen_actions_layout->addWidget(btn, 1);
        };
        add_screen_btn(QStringLiteral("Full-Screen"), QStringLiteral("output-fullscreen-btn"), [this] {
            if (!show_screen_handler_) {
                return;
            }
            const auto screens = QGuiApplication::screens();
            const int row = screens_list_->currentRow();
            QScreen* target = (row >= 0 && row < screens.size())
                                  ? screens.at(row)
                                  : (screens.size() > 1 ? screens.at(1) : QGuiApplication::primaryScreen());
            if (target != nullptr) {
                show_screen_handler_(target);
            }
        });
        add_screen_btn(QStringLiteral("Window"), QStringLiteral("output-window-btn"), [this] {
            if (show_movable_handler_) {
                show_movable_handler_();
            }
        });
        add_screen_btn(QStringLiteral("Hide"), QStringLiteral("output-hide-btn"), [this] {
            if (hide_output_handler_) {
                hide_output_handler_();
            }
        });
        screens_layout->addWidget(screen_actions);

        screens_layout->addWidget(make_section_header(QStringLiteral("RESOURCE GOVERNOR"), screens_page));
        auto* mem_panel = new QWidget{screens_page};
        mem_panel->setObjectName(QStringLiteral("memory-panel"));
        auto* mem_layout = new QVBoxLayout(mem_panel);
        mem_layout->setContentsMargins(10, 8, 10, 10);
        mem_layout->setSpacing(6);

        memory_status_label_ = new QLabel{
            QStringLiteral("Memory: -- / 270 MB · Live Video Admitted"), mem_panel};
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
        screens_scroll->setWidget(screens_page);
        drawer_stack_->addWidget(screens_scroll);

        // --- Page 2: Active Item Cues + Filter ---
        auto* cues_page = new QWidget{drawer_stack_};
        cues_page->setObjectName(QStringLiteral("drawer-page-cues"));
        auto* cues_layout = new QVBoxLayout(cues_page);
        cues_layout->setContentsMargins(0, 0, 0, 0);
        cues_layout->setSpacing(0);

        cues_layout->addWidget(
            make_section_header(QStringLiteral("0 CUES"), cues_page, nullptr, &cues_count_label_));

        cues_list_ = new QListWidget{cues_page};
        cues_list_->setObjectName(QStringLiteral("quick-cues-list"));
        cues_list_->setMinimumHeight(36);
        cues_list_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Ignored);
        cues_list_->setSelectionMode(QAbstractItemView::SingleSelection);
        QObject::connect(cues_list_, &QListWidget::itemClicked, this, [this](QListWidgetItem* item) {
            if (item == nullptr || !take_cue_handler_) {
                return;
            }
            const auto id = item->data(Qt::UserRole).toString();
            if (!id.isEmpty()) {
                take_cue_handler_(domain::CueId{id.toStdString()});
            }
        });
        cues_layout->addWidget(cues_list_, 1);

        auto* filter_bar = new QWidget{cues_page};
        filter_bar->setObjectName(QStringLiteral("cues-filter-bar"));
        auto* filter_layout = new QHBoxLayout(filter_bar);
        filter_layout->setContentsMargins(8, 5, 8, 5);
        filter_layout->setSpacing(6);

        cues_filter_input_ = new QLineEdit{filter_bar};
        cues_filter_input_->setObjectName(QStringLiteral("quick-cues-filter"));
        cues_filter_input_->setMinimumWidth(60);
        cues_filter_input_->setPlaceholderText(QStringLiteral("Filter cues..."));
        cues_filter_input_->setClearButtonEnabled(true);
        QObject::connect(cues_filter_input_, &QLineEdit::textChanged, this, [this] {
            apply_cue_filter();
        });
        filter_layout->addWidget(cues_filter_input_, 1);

        auto* shortcut_hint = new ElidedLabel{QStringLiteral("PgUp/Dn · Esc · B"), filter_bar};
        shortcut_hint->setObjectName(QStringLiteral("quick-cues-shortcut-hint"));
        filter_layout->addWidget(shortcut_hint);
        cues_layout->addWidget(filter_bar);
        drawer_stack_->addWidget(cues_page);

        layout->addWidget(drawer_stack_, 1);

}

}  // namespace shekinah::ui
