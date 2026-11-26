#include "shekinah/ui/support/theme.hpp"

namespace shekinah::ui {

QPalette ThemeManager::build_qpalette(const ThemePalette& p) {
    QPalette pal;
    pal.setColor(QPalette::Window, p.window_bg);
    pal.setColor(QPalette::WindowText, p.text_primary);
    pal.setColor(QPalette::Base, p.input_bg);
    pal.setColor(QPalette::AlternateBase, p.panel_alt_bg);
    pal.setColor(QPalette::ToolTipBase, p.menu_bg);
    pal.setColor(QPalette::ToolTipText, p.text_primary);
    pal.setColor(QPalette::Text, p.text_primary);
    pal.setColor(QPalette::PlaceholderText, p.text_subtle);
    pal.setColor(QPalette::Button, p.button_bg);
    pal.setColor(QPalette::ButtonText, p.text_primary);
    pal.setColor(QPalette::BrightText, p.text_selected);
    pal.setColor(QPalette::Highlight, p.accent);
    pal.setColor(QPalette::HighlightedText, p.text_on_accent);
    pal.setColor(QPalette::Link, p.text_link);
    pal.setColor(QPalette::Mid, p.border_subtle);
    pal.setColor(QPalette::Dark, p.border_strong);
    pal.setColor(QPalette::Shadow, p.border_strong);

    pal.setColor(QPalette::Disabled, QPalette::WindowText, p.text_disabled);
    pal.setColor(QPalette::Disabled, QPalette::Text, p.text_disabled);
    pal.setColor(QPalette::Disabled, QPalette::ButtonText, p.text_disabled);
    pal.setColor(QPalette::Disabled, QPalette::Button, p.button_disabled_bg);
    return pal;
}

}  // namespace shekinah::ui
