#include "shekinah/ui/program_status_widget.hpp"

#include "program_status_icons.hpp"
#include "program_status_theme_styles.hpp"

#include "shekinah/ui/support/theme.hpp"

#include <QPushButton>

namespace shekinah::ui {

void ProgramStatusWidget::apply_theme() {
    const auto& p = ThemeManager::instance().palette();
    setStyleSheet(program_status_internal::build_program_status_stylesheet(p));

    quick_clear_button_->setIcon(
        program_status::make_glyph_icon(program_status::IconGlyph::clear_all, p.quick_clear_fg, 12));
    screen_menu_button_->setIcon(
        program_status::make_glyph_icon(program_status::IconGlyph::chevron_down, p.icon_muted, 12));
    previous_button_->setIcon(
        program_status::make_glyph_icon(program_status::IconGlyph::previous, p.icon_neutral, 11));
    next_button_->setIcon(
        program_status::make_glyph_icon(program_status::IconGlyph::next, p.icon_neutral, 11));
    clear_button_->setIcon(
        program_status::make_glyph_icon(program_status::IconGlyph::clear_layer, p.icon_neutral, 11));
    if (next_up_header_button_ != nullptr) {
        next_up_header_button_->setIcon(program_status::make_glyph_icon(
            program_status::IconGlyph::arrow_right, p.icon_muted, 13));
    }

    set_active_tab(active_tab_);
    set_live_details(live_details_);
    set_memory_status(last_working_set_bytes_, last_live_media_admitted_);
    if (slide_preview_ != nullptr) {
        slide_preview_->update();
    }
}

}  // namespace shekinah::ui
