#include "shekinah/ui/program_status_widget.hpp"

#include "program_status_components.hpp"

namespace shekinah::ui {

ProgramStatusWidget::ProgramStatusWidget(QWidget* parent) : QWidget(parent) {
    setObjectName(QStringLiteral("program-status"));

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // 1. Compact Header Bar ("PROGRAM STATUS" + mode pill)
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

    // 2. Monitor + Circular Clear-All + Vertical Quick-Action Rail
    auto* monitor_row = new QWidget{this};
    monitor_row->setObjectName(QStringLiteral("monitor-row"));
    auto* monitor_layout = new QHBoxLayout(monitor_row);
    monitor_layout->setContentsMargins(6, 6, 0, 6);
    monitor_layout->setSpacing(4);

    auto* preview = new ProgramSlidePreview{monitor_row};
    preview->setObjectName(QStringLiteral("program-preview"));
    slide_preview_ = preview;
    monitor_layout->addWidget(preview, 1);

    // Floating-style circular Clear All button between preview and rail (ProPresenter hallmark)
    auto* clear_gutter = new QWidget{monitor_row};
    clear_gutter->setObjectName(QStringLiteral("clear-gutter"));
    clear_gutter->setFixedWidth(22);
    auto* gutter_layout = new QVBoxLayout(clear_gutter);
    gutter_layout->setContentsMargins(0, 0, 0, 0);
    gutter_layout->addStretch(1);
    quick_clear_button_ = new QPushButton{clear_gutter};
    quick_clear_button_->setObjectName(QStringLiteral("quick-clear-all"));
    quick_clear_button_->setFixedSize(20, 20);
    quick_clear_button_->setCursor(Qt::PointingHandCursor);
    quick_clear_button_->setToolTip(QStringLiteral("Clear All Output (Esc)"));
    quick_clear_button_->setIconSize(QSize{12, 12});
    QObject::connect(quick_clear_button_, &QPushButton::clicked, this, [this] {
        if (clear_handler_) {
            clear_handler_();
        }
    });
    gutter_layout->addWidget(quick_clear_button_, 0, Qt::AlignCenter);
    gutter_layout->addStretch(1);
    monitor_layout->addWidget(clear_gutter);

    // Right vertical quick-action rail
    auto* rail = new QWidget{monitor_row};
    rail->setObjectName(QStringLiteral("program-action-rail"));
    rail->setFixedWidth(34);
    auto* rail_layout = new QVBoxLayout(rail);
    rail_layout->setContentsMargins(0, 0, 0, 0);
    rail_layout->setSpacing(0);

    const auto add_rail_button = [&](const QString& name,
                                     const QString& tooltip,
                                     bool checkable,
                                     auto handler) -> QPushButton* {
        auto* btn = new QPushButton{rail};
        btn->setObjectName(name);
        btn->setProperty("rail", true);
        btn->setCheckable(checkable);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setToolTip(tooltip);
        btn->setFixedSize(34, 28);
        btn->setIconSize(QSize{15, 15});
        QObject::connect(btn, &QPushButton::clicked, this, handler);
        rail_layout->addWidget(btn);
        return btn;
    };

    rail_clear_button_ = add_rail_button(
        QStringLiteral("rail-clear-cue"), QStringLiteral("Clear Live Slide (Esc)"),
        false, [this] {
            if (clear_handler_) {
                clear_handler_();
            }
        });
    rail_background_button_ = add_rail_button(
        QStringLiteral("rail-background"), QStringLiteral("Set Background on Live Cue"),
        false, [this] {
            if (choose_background_handler_) {
                choose_background_handler_();
            }
        });
    rail_black_button_ = add_rail_button(
        QStringLiteral("rail-blackout"), QStringLiteral("Blackout Audience Output (B)"),
        true, [this] {
            if (black_handler_) {
                black_handler_();
            }
        });
    rail_audience_button_ = add_rail_button(
        QStringLiteral("rail-audience"), QStringLiteral("Show Audience Output Window"),
        true, [this] {
            if (output_visible_) {
                if (hide_output_handler_) {
                    hide_output_handler_();
                }
            } else if (show_movable_handler_) {
                show_movable_handler_();
            }
        });
    rail_inspector_button_ = add_rail_button(
        QStringLiteral("rail-inspector"), QStringLiteral("Open Slide Inspector"),
        false, [this] {
            if (inspect_live_handler_) {
                inspect_live_handler_();
            }
        });
    rail_layout->addStretch(1);
    monitor_layout->addWidget(rail);
    layout->addWidget(monitor_row);

    // 3. Screen Selector Bar ("Output: Monitor 2 v  |  Audience full-screen")
    auto* screen_bar = new QWidget{this};
    screen_bar->setObjectName(QStringLiteral("screen-selector-bar"));
    screen_bar->setFixedHeight(26);
    auto* screen_layout = new QHBoxLayout(screen_bar);
    screen_layout->setContentsMargins(8, 0, 8, 0);
    screen_layout->setSpacing(4);

    output_target_label_ = new ElidedLabel{QStringLiteral("Output: Monitor 2"), screen_bar};
    output_target_label_->setObjectName(QStringLiteral("output-target"));
    screen_layout->addWidget(output_target_label_);

    screen_menu_button_ = new QPushButton{screen_bar};
    screen_menu_button_->setObjectName(QStringLiteral("screen-menu-button"));
    screen_menu_button_->setFixedSize(18, 18);
    screen_menu_button_->setCursor(Qt::PointingHandCursor);
    screen_menu_button_->setToolTip(QStringLiteral("Select Audience Output Display"));
    screen_menu_button_->setIconSize(QSize{12, 12});
    QObject::connect(screen_menu_button_, &QPushButton::clicked, this, [this] { open_screen_menu(); });
    screen_layout->addWidget(screen_menu_button_);
    screen_layout->addStretch(1);

    audience_mode_label_ = new ElidedLabel{QStringLiteral("Audience full-screen"), screen_bar};
    audience_mode_label_->setObjectName(QStringLiteral("audience-mode"));
    audience_mode_label_->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    screen_layout->addWidget(audience_mode_label_);
    layout->addWidget(screen_bar);

    // 4. Layer Status Strip + Live Cue Identifier (cue_label_ MUST stay above global_controls_)
    auto* status_strip = new QWidget{this};
    status_strip->setObjectName(QStringLiteral("program-status-strip"));
    auto* status_layout = new QVBoxLayout(status_strip);
    status_layout->setContentsMargins(8, 6, 8, 6);
    status_layout->setSpacing(4);

    auto* indicator_row = new QHBoxLayout;
    indicator_row->setSpacing(10);
    indicator_row->addStretch(1);
    const auto make_indicator = [&](const QString& name, const QString& tip) {
        auto* btn = new QPushButton{status_strip};
        btn->setObjectName(name);
        btn->setProperty("statusIndicator", true);
        btn->setToolTip(tip);
        btn->setFixedSize(26, 20);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setIconSize(QSize{14, 14});
        indicator_row->addWidget(btn);
        return btn;
    };
    status_screen_indicator_ = make_indicator(
        QStringLiteral("indicator-screen"), QStringLiteral("Slide Layer Status"));
    status_media_indicator_ = make_indicator(
        QStringLiteral("indicator-media"), QStringLiteral("Background Media Status"));
    status_audience_indicator_ = make_indicator(
        QStringLiteral("indicator-audience"), QStringLiteral("Toggle Audience Output Window"));
    QObject::connect(status_screen_indicator_, &QPushButton::clicked, this, [this] {
        set_active_tab(0);
    });
    QObject::connect(status_media_indicator_, &QPushButton::clicked, this, [this] {
        if (choose_background_handler_) {
            choose_background_handler_();
        }
    });
    QObject::connect(status_audience_indicator_, &QPushButton::clicked, this, [this] {
        set_active_tab(1);
    });
    indicator_row->addStretch(1);
    status_layout->addLayout(indicator_row);

    cue_label_ = new ElidedLabel{QStringLiteral("No cue live"), status_strip};
    cue_label_->setObjectName(QStringLiteral("program-cue"));
    cue_label_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    cue_label_->setAlignment(Qt::AlignCenter);
    status_layout->addWidget(cue_label_);
    layout->addWidget(status_strip);

    // 5. Slide Progress Track + Transport Controls (global-controls)
    slide_progress_ = new QProgressBar{this};
    slide_progress_->setObjectName(QStringLiteral("slide-progress"));
    slide_progress_->setTextVisible(false);
    slide_progress_->setFixedHeight(3);
    slide_progress_->setRange(0, 100);
    slide_progress_->setValue(0);
    layout->addWidget(slide_progress_);

    global_controls_ = new QWidget{this};
    global_controls_->setObjectName(QStringLiteral("global-controls"));
    auto* controls = new QHBoxLayout(global_controls_);
    controls->setContentsMargins(6, 6, 6, 8);
    controls->setSpacing(4);

    const auto add_control = [&](const QString& label,
                                 const QString& object_name,
                                 auto handler) -> QPushButton* {
        auto* button = new QPushButton{label, global_controls_};
        button->setObjectName(object_name);
        button->setCursor(Qt::PointingHandCursor);
        button->setIconSize(QSize{11, 11});
        QObject::connect(button, &QPushButton::clicked, this, handler);
        controls->addWidget(button, 1);
        return button;
    };

    previous_button_ = add_control(
        QStringLiteral("Previous"), QStringLiteral("previous"), [this] {
            if (previous_handler_) {
                previous_handler_();
            }
        });
    next_button_ = add_control(
        QStringLiteral("Next"), QStringLiteral("next"), [this] {
            if (next_handler_) {
                next_handler_();
            }
        });
    clear_button_ = add_control(
        QStringLiteral("Clear"), QStringLiteral("clear"), [this] {
            if (clear_handler_) {
                clear_handler_();
            }
        });
    black_button_ = add_control(
        QStringLiteral("Black"), QStringLiteral("black"), [this] {
            if (black_handler_) {
                black_handler_();
            }
        });
    layout->addWidget(global_controls_);

    // 6. ProPresenter Segmented Icon Tab Bar
    auto* tab_bar = new QWidget{this};
    tab_bar->setObjectName(QStringLiteral("drawer-tab-bar"));
    tab_bar->setFixedHeight(28);
    auto* tab_layout = new QHBoxLayout(tab_bar);
    tab_layout->setContentsMargins(0, 0, 0, 0);
    tab_layout->setSpacing(1);

    const auto add_tab = [&](const QString& name, const QString& tip, int idx) {
        auto* btn = new QPushButton{tab_bar};
        btn->setObjectName(name);
        btn->setToolTip(tip);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setCheckable(true);
        btn->setFixedHeight(27);
        btn->setIconSize(QSize{15, 15});
        QObject::connect(btn, &QPushButton::clicked, this, [this, idx] { set_active_tab(idx); });
        tab_layout->addWidget(btn, 1);
        return btn;
    };
    tab_live_button_ = add_tab(
        QStringLiteral("tab-live"), QStringLiteral("Live & Next Slide"), 0);
    tab_screens_button_ = add_tab(
        QStringLiteral("tab-screens"), QStringLiteral("Screens & Diagnostics"), 1);
    tab_cues_button_ = add_tab(
        QStringLiteral("tab-cues"), QStringLiteral("Active Item Cues"), 2);
    layout->addWidget(tab_bar);

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

    refresh_screens_list();
    set_active_tab(0);
    apply_theme();
    QObject::connect(&ThemeManager::instance(), &ThemeManager::theme_changed, this,
                     [this](ThemeId) { apply_theme(); });
}

}  // namespace shekinah::ui
