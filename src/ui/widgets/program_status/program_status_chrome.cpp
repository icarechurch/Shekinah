#include "shekinah/ui/program_status_widget.hpp"

#include "program_status_icons.hpp"

#include "shekinah/ui/support/theme.hpp"

#include <QLabel>
#include <QPushButton>

namespace shekinah::ui {

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
