#include "theme_palettes.hpp"

namespace shekinah::ui::theme_internal {

ThemePalette make_warm_light_palette() {
    ThemePalette p = make_light_palette();
    p.id = ThemeId::warm_light;
    p.is_dark = false;

    p.window_bg = QColor{QStringLiteral("#ede6da")};
    p.panel_bg = QColor{QStringLiteral("#f6f2ea")};
    p.panel_alt_bg = QColor{QStringLiteral("#f3eee4")};
    p.status_panel_bg = QColor{QStringLiteral("#f6f2ea")};
    p.header_bg = QColor{QStringLiteral("#e5ddd0")};
    p.sub_header_bg = QColor{QStringLiteral("#e8e1d5")};
    p.toolbar_bg = QColor{QStringLiteral("#efe9de")};
    p.menubar_bg = QColor{QStringLiteral("#f4efe6")};
    p.filter_bar_bg = QColor{QStringLiteral("#ebe4d8")};
    p.input_bg = QColor{QStringLiteral("#fffdf9")};
    p.input_alt_bg = QColor{QStringLiteral("#fffdf9")};
    p.card_bg = QColor{QStringLiteral("#fcfaf5")};
    p.card_hover_bg = QColor{QStringLiteral("#f1eadf")};
    p.card_selected_bg = QColor{QStringLiteral("#d8ebe4")};
    p.card_drop_bg = QColor{QStringLiteral("#d4eaf5")};
    p.list_bg = QColor{QStringLiteral("#fcfaf5")};
    p.list_hover_bg = QColor{QStringLiteral("#f0e9dd")};
    p.list_selected_bg = QColor{QStringLiteral("#d8ebe4")};
    p.order_selected_bg = QColor{QStringLiteral("#d8ebe4")};
    p.menu_bg = QColor{QStringLiteral("#fcfaf5")};
    p.menu_hover_bg = QColor{QStringLiteral("#e8e0d2")};

    p.border_strong = QColor{QStringLiteral("#cfc5b4")};
    p.border_mid = QColor{QStringLiteral("#d6cdbd")};
    p.border_subtle = QColor{QStringLiteral("#ddd5c7")};
    p.border_card = QColor{QStringLiteral("#d6cdbd")};
    p.border_divider = QColor{QStringLiteral("#e8e1d4")};
    p.border_hover = QColor{QStringLiteral("#9c907d")};
    p.scrollbar_track = QColor{QStringLiteral("#ebe4d8")};
    p.scrollbar_handle = QColor{QStringLiteral("#c5bba8")};
    p.scrollbar_handle_hover = QColor{QStringLiteral("#9e927f")};

    p.text_primary = QColor{QStringLiteral("#262018")};
    p.text_secondary = QColor{QStringLiteral("#3e352b")};
    p.text_muted = QColor{QStringLiteral("#695e50")};
    p.text_subtle = QColor{QStringLiteral("#857969")};
    p.text_disabled = QColor{QStringLiteral("#ada293")};
    p.text_selected = QColor{QStringLiteral("#132620")};
    p.text_selected_sub = QColor{QStringLiteral("#0f6e5b")};
    p.text_link = QColor{QStringLiteral("#0f766e")};
    p.text_link_hover = QColor{QStringLiteral("#0a524c")};

    p.button_bg = QColor{QStringLiteral("#fcfaf5")};
    p.button_alt_bg = QColor{QStringLiteral("#fcfaf5")};
    p.button_border = QColor{QStringLiteral("#cfc5b4")};
    p.button_alt_border = QColor{QStringLiteral("#cfc5b4")};
    p.button_hover_bg = QColor{QStringLiteral("#efe8db")};
    p.button_hover_border = QColor{QStringLiteral("#9c907d")};
    p.button_pressed_bg = QColor{QStringLiteral("#e4dccb")};
    p.button_disabled_bg = QColor{QStringLiteral("#f1ece2")};
    p.button_disabled_border = QColor{QStringLiteral("#e0d8ca")};
    p.badge_bg = QColor{QStringLiteral("#e4dccd")};
    p.badge_border = QColor{QStringLiteral("#cfc5b4")};
    p.badge_fg = QColor{QStringLiteral("#52483c")};
    p.tab_bar_bg = QColor{QStringLiteral("#e5ddd0")};
    p.tab_inactive_bg = QColor{QStringLiteral("#eee8dc")};
    p.tab_inactive_fg = QColor{QStringLiteral("#473d32")};
    p.tab_active_bg = QColor{QStringLiteral("#0f766e")};
    p.rail_bg = QColor{QStringLiteral("#ece5d8")};
    p.rail_hover_bg = QColor{QStringLiteral("#dfd7c7")};
    p.rail_border = QColor{QStringLiteral("#d6cdbd")};
    p.rail_icon = QColor{QStringLiteral("#473d32")};
    p.icon_neutral = QColor{QStringLiteral("#473d32")};
    p.icon_muted = QColor{QStringLiteral("#695e50")};
    p.icon_dim = QColor{QStringLiteral("#857969")};
    p.icon_empty = QColor{QStringLiteral("#a39787")};
    p.quick_clear_bg = QColor{QStringLiteral("#2c251d")};
    p.quick_clear_border = QColor{QStringLiteral("#1f1a14")};
    p.quick_clear_hover_bg = QColor{QStringLiteral("#42382c")};
    p.quick_clear_pressed_bg = QColor{QStringLiteral("#1a1510")};

    p.accent = QColor{QStringLiteral("#0f766e")};
    p.accent_hover = QColor{QStringLiteral("#0d665f")};
    p.accent_pressed = QColor{QStringLiteral("#0a524c")};
    p.accent_border = QColor{QStringLiteral("#0d665f")};
    p.accent_hover_border = QColor{QStringLiteral("#0a524c")};
    p.drop_highlight = QColor{QStringLiteral("#0284c7")};

    p.live_fg = QColor{QStringLiteral("#0f766e")};
    p.live_pill_bg = QColor{15, 118, 110, 32};
    p.live_pill_border = QColor{15, 118, 110, 125};
    p.live_clear_bg = QColor{QStringLiteral("#e5ddd0")};
    p.live_clear_border = QColor{QStringLiteral("#cbc0ad")};

    p.add_pill_bg = QColor{239, 233, 222, 235};
    p.add_pill_border = QColor{163, 151, 135, 200};
    p.add_pill_fg = QColor{QStringLiteral("#2c251d")};
    p.add_pill_selected_bg = QColor{15, 118, 110, 38};
    p.add_pill_selected_border = QColor{15, 118, 110, 135};
    p.add_pill_selected_fg = QColor{QStringLiteral("#0a524c")};

    p.cue_card_bg = QColor{QStringLiteral("#dfd7c8")};
    p.cue_card_border = QColor{QStringLiteral("#c4b9a6")};
    p.thumb_well_border = QColor{QStringLiteral("#c8beac")};
    p.monitor_row_bg = QColor{QStringLiteral("#e3dbcd")};
    return p;
}

}  // namespace shekinah::ui::theme_internal
