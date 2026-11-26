#include "program_status_theme_styles.hpp"

namespace shekinah::ui::program_status_internal {

QString build_program_status_stylesheet(const ThemePalette& p) {
    return QStringLiteral(R"(
#program-status { background-color: %1; border-left: 1px solid %2; color: %3; }
#program-header-bar { background-color: %4; border-bottom: 1px solid %2; }
#program-status-title { color: %5; font-size: 10px; font-weight: 700; letter-spacing: 1.1px; border: none; background: transparent; }
#monitor-row { background-color: %6; }
#clear-gutter { background: transparent; }
QPushButton#quick-clear-all { background-color: %7; border: 1px solid %8; border-radius: 10px; }
QPushButton#quick-clear-all:hover { background-color: %9; }
QPushButton#quick-clear-all:pressed { background-color: %10; }
#program-action-rail { background-color: %11; border-left: 1px solid %2; }
#program-status QPushButton[rail="true"] { background-color: %11; border: none; border-bottom: 1px solid %12; border-radius: 0px; padding: 0px; }
#program-status QPushButton[rail="true"]:hover { background-color: %13; }
#program-status QPushButton[rail="true"]:checked { background-color: %14; }
#screen-selector-bar { background-color: %15; border-top: 1px solid %16; border-bottom: 1px solid %16; }
#output-target { color: %17; font-size: 11px; font-weight: 600; border: none; background: transparent; }
#audience-mode { color: %5; font-size: 10px; border: none; background: transparent; }
#screen-menu-button { background: transparent; border: none; border-radius: 3px; }
#screen-menu-button:hover { background: %18; }
#program-status-strip { background-color: %19; }
#program-status QPushButton[statusIndicator="true"] { background: transparent; border: none; border-radius: 3px; }
#program-status QPushButton[statusIndicator="true"]:hover { background: %18; }
#slide-progress { background-color: %20; border: none; }
#slide-progress::chunk { background-color: %14; }
#global-controls { background-color: %19; border-bottom: 1px solid %2; }
#program-status QWidget#global-controls QPushButton { background-color: %21; border: 1px solid %22; border-radius: 4px; color: %3; font-size: 10px; font-weight: 600; padding: 4px 4px; min-height: 24px; }
#program-status QWidget#global-controls QPushButton:hover { background-color: %23; border-color: %24; }
#program-status QWidget#global-controls QPushButton:pressed { background-color: %25; }
#drawer-tab-bar { background-color: %26; border-bottom: 1px solid %2; }
#drawer-page-live-scroll, #drawer-page-live, #drawer-page-screens-scroll, #drawer-page-screens, #drawer-page-cues { background-color: %27; }
#program-status QWidget[sectionHeader="true"] { background-color: %28; border-top: 1px solid %16; border-bottom: 1px solid %16; }
#program-status QLabel[sectionTitle="true"] { color: %17; font-size: 10px; font-weight: 700; letter-spacing: 0.8px; border: none; background: transparent; }
#program-status QPushButton[linkAction="true"] { background: transparent; border: none; color: %29; font-size: 10px; font-weight: 600; padding: 2px 4px; }
#program-status QPushButton[linkAction="true"]:hover { color: %30; }
#live-card, #memory-panel { background-color: %4; }
#next-up-card { background-color: %31; }
#screen-actions-bar { background-color: %32; }
#live-item-title { color: %3; font-size: 11px; font-weight: 600; background: transparent; }
#live-slide-counter, #live-asset-label { color: %5; font-size: 10px; font-weight: 600; background: transparent; }
#live-lyrics-preview { color: %17; font-size: 11px; background-color: %20; border: 1px solid %33; border-radius: 4px; padding: 6px 8px; }
#live-set-bg-button { background: %21; border: 1px solid %22; border-radius: 3px; color: %17; font-size: 10px; padding: 2px 7px; }
#live-set-bg-button:hover { background: %23; }
#next-up-header-button { background: transparent; border: none; padding: 2px; }
#next-up-header-button:hover { background: %18; border-radius: 3px; }
#next-summary-label { color: %5; font-size: 11px; background: transparent; }
#next-up-action-button { background-color: %21; border: 1px solid %22; border-radius: 5px; color: %3; font-size: 11px; font-weight: 600; padding: 6px 10px; }
#next-up-action-button:hover { background-color: %23; border-color: %14; }
#program-status QPushButton[screenAction="true"] { background: %21; border: 1px solid %22; border-radius: 4px; color: %3; font-size: 10px; font-weight: 600; padding: 4px 6px; }
#program-status QPushButton[screenAction="true"]:hover { background: %23; }
#memory-status-label { color: %17; font-size: 10px; background: transparent; }
#cues-filter-bar { background-color: %34; border-top: 1px solid %16; }
#quick-cues-shortcut-hint { color: %35; font-size: 10px; border: none; background: transparent; }
#program-status QListWidget { background-color: %32; border: none; color: %17; font-size: 11px; outline: 0; }
#program-status QListWidget::item { padding: 5px 8px; border-bottom: 1px solid %36; }
#program-status QListWidget::item:selected { background-color: %37; color: %38; }
#program-status QListWidget::item:hover { background-color: %39; }
#program-status QLineEdit { background-color: %20; border: 1px solid %33; border-radius: 4px; color: %3; font-size: 11px; padding: 3px 8px; }
#program-status QLineEdit:focus { border-color: %14; }
)")
        .arg(p.status_panel_bg.name(), p.border_strong.name(), p.text_primary.name(), p.header_bg.name(),
             p.text_muted.name(), p.monitor_row_bg.name(), p.quick_clear_bg.name(), p.quick_clear_border.name(),
             p.quick_clear_hover_bg.name(), p.quick_clear_pressed_bg.name(), p.rail_bg.name(), p.rail_border.name(),
             p.rail_hover_bg.name(), p.accent.name(), p.toolbar_bg.name(), p.border_mid.name(), p.text_secondary.name(),
             p.menu_hover_bg.name(), p.toolbar_bg.name(), p.input_alt_bg.name(), p.button_bg.name(), p.button_border.name(),
             p.button_hover_bg.name(), p.button_hover_border.name(), p.button_pressed_bg.name(), p.tab_bar_bg.name(),
             p.panel_alt_bg.name(), p.sub_header_bg.name(), p.text_link.name(), p.text_link_hover.name(), p.panel_bg.name(),
             p.list_bg.name(), p.border_subtle.name(), p.filter_bar_bg.name(), p.text_subtle.name(), p.border_divider.name(),
             p.list_selected_bg.name(), p.text_selected.name(), p.list_hover_bg.name());
}

}  // namespace shekinah::ui::program_status_internal
