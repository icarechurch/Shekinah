#include "shekinah/ui/program_status_widget.hpp"

#include "shekinah/ui/support/elided_label.hpp"

#include <QHBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QVBoxLayout>

namespace shekinah::ui {

void ProgramStatusWidget::build_screen_selector(QVBoxLayout* layout) {
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
}

void ProgramStatusWidget::build_status_strip(QVBoxLayout* layout) {
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
}

void ProgramStatusWidget::build_global_controls(QVBoxLayout* layout) {
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
}

}  // namespace shekinah::ui
