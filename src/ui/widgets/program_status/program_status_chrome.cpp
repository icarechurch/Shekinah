#include "shekinah/ui/program_status_widget.hpp"

#include "program_status_icons.hpp"
#include "program_status_preview.hpp"

#include "shekinah/ui/support/elided_label.hpp"
#include "shekinah/ui/support/theme.hpp"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSizePolicy>
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

void ProgramStatusWidget::update_status_chrome() {
    const auto& p = ThemeManager::instance().palette();
    const auto rgba_css = [](const QColor& c) {
        return QStringLiteral("rgba(%1, %2, %3, %4)")
            .arg(c.red())
            .arg(c.green())
            .arg(c.blue())
            .arg(c.alphaF(), 0, 'f', 2);
    };

    switch (current_program_.mode) {
    case domain::ProgramMode::cue:
        mode_label_->setStyleSheet(
            QStringLiteral("background-color: %1; color: %2; border: 1px solid %3; border-radius: 3px; padding: 1px 6px; font-size: 10px; font-weight: 700;")
                .arg(rgba_css(p.live_pill_bg), p.live_fg.name(), rgba_css(p.live_pill_border)));
        cue_label_->setStyleSheet(
            QStringLiteral("color: %1; font-size: 11px; font-weight: 600; background: transparent;")
                .arg(p.text_primary.name()));
        break;
    case domain::ProgramMode::black:
        mode_label_->setStyleSheet(
            QStringLiteral("background-color: %1; color: %2; border: 1px solid %3; border-radius: 3px; padding: 1px 6px; font-size: 10px; font-weight: 700;")
                .arg(rgba_css(p.danger_pill_bg), p.danger_soft_fg.name(), rgba_css(p.danger_pill_border)));
        cue_label_->setStyleSheet(
            QStringLiteral("color: %1; font-size: 11px; font-weight: 600; background: transparent;")
                .arg(p.danger_soft_fg.name()));
        break;
    case domain::ProgramMode::clear:
        mode_label_->setStyleSheet(
            QStringLiteral("background-color: %1; color: %2; border: 1px solid %3; border-radius: 3px; padding: 1px 6px; font-size: 10px; font-weight: 700;")
                .arg(p.live_clear_bg.name(), p.live_fg.name(), p.live_clear_border.name()));
        cue_label_->setStyleSheet(
            QStringLiteral("color: %1; font-size: 11px; font-weight: 500; background: transparent;")
                .arg(p.live_fg.name()));
        break;
    }

    const bool is_live = current_program_.mode == domain::ProgramMode::cue;
    const bool is_black = current_program_.mode == domain::ProgramMode::black;
    rail_black_button_->setChecked(is_black);
    rail_audience_button_->setChecked(output_visible_);

    rail_clear_button_->setIcon(
        program_status::make_glyph_icon(program_status::IconGlyph::slide, p.rail_icon, 15));
    rail_background_button_->setIcon(
        program_status::make_glyph_icon(program_status::IconGlyph::media_image, p.rail_icon, 15));
    rail_black_button_->setIcon(
        program_status::make_glyph_icon(
            program_status::IconGlyph::blackout, is_black ? p.text_on_accent : p.rail_icon, 15));
    rail_audience_button_->setIcon(
        program_status::make_glyph_icon(
            program_status::IconGlyph::display, output_visible_ ? p.text_on_accent : p.rail_icon, 15));
    rail_inspector_button_->setIcon(
        program_status::make_glyph_icon(program_status::IconGlyph::sliders, p.rail_icon, 15));

    status_screen_indicator_->setIcon(program_status::make_glyph_icon(
        program_status::IconGlyph::slide,
        is_live ? p.accent : is_black ? p.danger_fg : p.icon_dim,
        14));
    status_media_indicator_->setIcon(program_status::make_glyph_icon(
        program_status::IconGlyph::media_image,
        (is_live && live_details_.has_background) ? p.accent : p.icon_dim,
        14));
    status_audience_indicator_->setIcon(program_status::make_glyph_icon(
        program_status::IconGlyph::display,
        output_visible_ ? p.accent : p.icon_dim,
        14));

    black_button_->setStyleSheet(
        is_black
            ? QStringLiteral("background-color: %1; border-color: %2; color: #ffffff;")
                  .arg(p.danger_bg.name(), p.danger_border.name())
            : QString{});
    black_button_->setIcon(
        program_status::make_glyph_icon(
            program_status::IconGlyph::blackout, is_black ? QColor{Qt::white} : p.icon_neutral, 11));
}

}  // namespace shekinah::ui
