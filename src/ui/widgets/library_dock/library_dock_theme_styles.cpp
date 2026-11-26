#include "library_dock_theme_styles.hpp"

namespace shekinah::ui::library_internal {

QString build_library_dock_stylesheet(const ThemePalette& p) {
    return QStringLiteral(R"(
#library-dock { background-color: %1; color: %2; }
#library-dock-header { background-color: %3; border-top: 1px solid %4; border-bottom: 1px solid %4; }
#library-dock-title { color: %5; font-size: 10px; font-weight: 700; letter-spacing: 1.1px; border: none; background: transparent; }
#library-total-count { background-color: %6; color: %7; border: 1px solid %8; border-radius: 3px; padding: 0px 6px; font-size: 9px; font-weight: 700; }
#library-dock-hint { color: %9; font-size: 10px; border: none; background: transparent; }
#library-float-toggle { background: transparent; border: none; border-radius: 3px; }
#library-float-toggle:hover { background-color: %10; }
#library-toolbar { background-color: %11; border-bottom: 1px solid %12; }
#library-tabs, #asset-filter-bar { background-color: %13; border: 1px solid %14; border-radius: 4px; }
#library-tabs QPushButton, #asset-filter-bar QPushButton { background: transparent; border: none; border-radius: 3px; color: %5; font-size: 11px; font-weight: 600; padding: 2px 9px; }
#library-tabs QPushButton:hover, #asset-filter-bar QPushButton:hover { background-color: %15; color: %2; }
#library-tabs QPushButton:checked { background-color: %16; color: %17; }
#asset-filter-bar QPushButton:checked { background-color: %18; color: %19; }
#library-dock QLineEdit#library-search { background-color: %13; border: 1px solid %14; border-radius: 4px; color: %2; font-size: 11px; padding: 3px 8px; }
#library-dock QLineEdit#library-search:focus { border-color: %16; }
#library-dock QPushButton[libAction="true"] { background-color: %20; border: 1px solid %21; border-radius: 4px; color: %22; font-size: 11px; font-weight: 600; padding: 3px 8px; }
#library-dock QPushButton[libAction="true"]:hover { background-color: %23; border-color: %24; }
#library-dock QPushButton[libAction="true"]:pressed { background-color: %25; }
#library-dock QPushButton[libAction="true"]:disabled { background-color: %26; border-color: %27; color: %28; }
#library-dock QPushButton[libAction="true"][primaryAction="true"] { background-color: %16; border-color: %29; color: %17; }
#library-dock QPushButton[libAction="true"][primaryAction="true"]:hover { background-color: %30; border-color: %31; }
#library-dock QPushButton[libAction="true"][primaryAction="true"]:pressed { background-color: %32; }
#library-dock QListWidget { background-color: %1; border: none; color: %2; outline: 0; padding: 6px; }
#library-dock QListWidget::item { background: transparent; border: none; padding: 0px; }
#library-empty-state { background: transparent; }
#library-empty-title { color: %22; font-size: 12px; font-weight: 600; background: transparent; }
#library-empty-hint { color: %9; font-size: 11px; background: transparent; }
)")
        .arg(p.window_bg.name(), p.text_primary.name(), p.header_bg.name(), p.border_strong.name(),
             p.text_muted.name(), p.badge_bg.name(), p.badge_fg.name(), p.badge_border.name(),
             p.text_subtle.name(), p.menu_hover_bg.name(), p.toolbar_bg.name(), p.border_mid.name(),
             p.input_bg.name(), p.border_subtle.name(), p.list_hover_bg.name(), p.accent.name(),
             p.text_on_accent.name(), p.button_hover_bg.name(), p.text_primary.name(), p.button_alt_bg.name(),
             p.button_alt_border.name(), p.text_secondary.name(), p.button_hover_bg.name(),
             p.button_hover_border.name(), p.button_pressed_bg.name(), p.button_disabled_bg.name(),
             p.button_disabled_border.name(), p.text_disabled.name(), p.accent_border.name(),
             p.accent_hover.name(), p.accent_hover_border.name(), p.accent_pressed.name());
}

}  // namespace shekinah::ui::library_internal
