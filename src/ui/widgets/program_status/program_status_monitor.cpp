#include "shekinah/ui/program_status_widget.hpp"

#include "program_status_preview.hpp"

#include "shekinah/ui/support/elided_label.hpp"

#include <QHBoxLayout>
#include <QPushButton>
#include <QVBoxLayout>

namespace shekinah::ui {

void ProgramStatusWidget::build_monitor(QVBoxLayout* layout) {
    auto* monitor_row = new QWidget{this};
    monitor_row->setObjectName(QStringLiteral("monitor-row"));
    auto* monitor_layout = new QHBoxLayout(monitor_row);
    monitor_layout->setContentsMargins(6, 6, 0, 6);
    monitor_layout->setSpacing(4);

    auto* preview = new ProgramSlidePreview{monitor_row};
    preview->setObjectName(QStringLiteral("program-preview"));
    slide_preview_ = preview;
    monitor_layout->addWidget(preview, 1);

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
        QStringLiteral("rail-clear-cue"), QStringLiteral("Clear Live Slide (Esc)"), false, [this] {
            if (clear_handler_) {
                clear_handler_();
            }
        });
    rail_background_button_ = add_rail_button(
        QStringLiteral("rail-background"), QStringLiteral("Set Background on Live Cue"), false, [this] {
            if (choose_background_handler_) {
                choose_background_handler_();
            }
        });
    rail_black_button_ = add_rail_button(
        QStringLiteral("rail-blackout"), QStringLiteral("Blackout Audience Output (B)"), true, [this] {
            if (black_handler_) {
                black_handler_();
            }
        });
    rail_audience_button_ = add_rail_button(
        QStringLiteral("rail-audience"), QStringLiteral("Show Audience Output Window"), true, [this] {
            if (output_visible_) {
                if (hide_output_handler_) {
                    hide_output_handler_();
                }
            } else if (show_movable_handler_) {
                show_movable_handler_();
            }
        });
    rail_inspector_button_ = add_rail_button(
        QStringLiteral("rail-inspector"), QStringLiteral("Open Slide Inspector"), false, [this] {
            if (inspect_live_handler_) {
                inspect_live_handler_();
            }
        });
    rail_layout->addStretch(1);
    monitor_layout->addWidget(rail);
    layout->addWidget(monitor_row);
}

}  // namespace shekinah::ui
