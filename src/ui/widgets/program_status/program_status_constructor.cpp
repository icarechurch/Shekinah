#include "shekinah/ui/program_status_widget.hpp"

#include "program_status_components.hpp"
#include "program_status_preview.hpp"

#include "shekinah/ui/support/elided_label.hpp"

#include <QHBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QVBoxLayout>

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

    build_drawer_pages(layout);

    refresh_screens_list();
    set_active_tab(0);
    apply_theme();
    QObject::connect(&ThemeManager::instance(), &ThemeManager::theme_changed, this,
                     [this](ThemeId) { apply_theme(); });
}

}  // namespace shekinah::ui
