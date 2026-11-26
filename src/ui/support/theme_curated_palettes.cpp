#include "theme_palettes.hpp"

namespace shekinah::ui::theme_internal {

ThemePalette make_midnight_palette() {
    ThemePalette p = make_dark_palette();
    p.id = ThemeId::midnight;
    p.is_dark = true;

    p.window_bg = QColor{QStringLiteral("#08090c")};
    p.panel_bg = QColor{QStringLiteral("#0b0d12")};
    p.panel_alt_bg = QColor{QStringLiteral("#0d0f15")};
    p.status_panel_bg = QColor{QStringLiteral("#0c0e13")};
    p.header_bg = QColor{QStringLiteral("#0a0b0f")};
    p.sub_header_bg = QColor{QStringLiteral("#141720")};
    p.toolbar_bg = QColor{QStringLiteral("#0f1117")};
    p.menubar_bg = QColor{QStringLiteral("#0c0e13")};
    p.filter_bar_bg = QColor{QStringLiteral("#0f1117")};
    p.input_bg = QColor{QStringLiteral("#060709")};
    p.input_alt_bg = QColor{QStringLiteral("#060709")};
    p.card_bg = QColor{QStringLiteral("#10131a")};
    p.card_hover_bg = QColor{QStringLiteral("#171c26")};
    p.card_selected_bg = QColor{QStringLiteral("#0f2744")};
    p.card_drop_bg = QColor{QStringLiteral("#0e293d")};
    p.list_bg = QColor{QStringLiteral("#0b0d12")};
    p.list_hover_bg = QColor{QStringLiteral("#141821")};
    p.list_selected_bg = QColor{QStringLiteral("#132c4d")};
    p.order_selected_bg = QColor{QStringLiteral("#102845")};
    p.menu_bg = QColor{QStringLiteral("#10131a")};
    p.menu_hover_bg = QColor{QStringLiteral("#1b202c")};

    p.border_strong = QColor{QStringLiteral("#161922")};
    p.border_mid = QColor{QStringLiteral("#181c26")};
    p.border_subtle = QColor{QStringLiteral("#1f2432")};
    p.border_card = QColor{QStringLiteral("#1f2432")};
    p.border_divider = QColor{QStringLiteral("#161a24")};
    p.border_hover = QColor{QStringLiteral("#333c52")};
    p.scrollbar_track = QColor{QStringLiteral("#08090c")};
    p.scrollbar_handle = QColor{QStringLiteral("#232836")};
    p.scrollbar_handle_hover = QColor{QStringLiteral("#374054")};

    p.text_primary = QColor{QStringLiteral("#dce1eb")};
    p.text_secondary = QColor{QStringLiteral("#c2c9d7")};
    p.text_muted = QColor{QStringLiteral("#788092")};
    p.text_subtle = QColor{QStringLiteral("#565e70")};
    p.text_disabled = QColor{QStringLiteral("#3c4250")};

    p.button_bg = QColor{QStringLiteral("#161922")};
    p.button_alt_bg = QColor{QStringLiteral("#141720")};
    p.button_border = QColor{QStringLiteral("#242a38")};
    p.button_alt_border = QColor{QStringLiteral("#222735")};
    p.button_hover_bg = QColor{QStringLiteral("#1e2330")};
    p.button_hover_border = QColor{QStringLiteral("#374054")};
    p.button_pressed_bg = QColor{QStringLiteral("#0f1218")};
    p.button_disabled_bg = QColor{QStringLiteral("#101218")};
    p.button_disabled_border = QColor{QStringLiteral("#181c26")};
    p.badge_bg = QColor{QStringLiteral("#161922")};
    p.badge_border = QColor{QStringLiteral("#242a38")};
    p.tab_bar_bg = QColor{QStringLiteral("#141720")};
    p.tab_inactive_bg = QColor{QStringLiteral("#181c26")};
    p.rail_bg = QColor{QStringLiteral("#0d0f15")};
    p.rail_hover_bg = QColor{QStringLiteral("#181c26")};
    p.rail_border = QColor{QStringLiteral("#191d28")};

    p.accent = QColor{QStringLiteral("#2563eb")};
    p.accent_hover = QColor{QStringLiteral("#3b82f6")};
    p.accent_pressed = QColor{QStringLiteral("#1d4ed8")};
    p.accent_border = QColor{QStringLiteral("#3b82f6")};
    p.accent_hover_border = QColor{QStringLiteral("#60a5fa")};
    p.tab_active_bg = QColor{QStringLiteral("#2563eb")};

    p.live_fg = QColor{QStringLiteral("#10b981")};
    p.live_pill_bg = QColor{16, 185, 129, 36};
    p.live_pill_border = QColor{16, 185, 129, 115};
    p.live_clear_bg = QColor{QStringLiteral("#151822")};
    p.live_clear_border = QColor{QStringLiteral("#242938")};

    p.cue_card_bg = QColor{QStringLiteral("#060709")};
    p.cue_card_border = QColor{QStringLiteral("#1e2330")};
    p.thumb_well_bg = QColor{QStringLiteral("#060709")};
    p.thumb_well_border = QColor{QStringLiteral("#1a1e29")};
    p.monitor_row_bg = QColor{QStringLiteral("#06070a")};
    p.preview_well_bg = QColor{QStringLiteral("#040507")};
    p.preview_black_bg = QColor{QStringLiteral("#020203")};
    return p;
}

ThemePalette make_slate_palette() {
    ThemePalette p = make_dark_palette();
    p.id = ThemeId::slate;
    p.is_dark = true;

    p.window_bg = QColor{QStringLiteral("#222834")};
    p.panel_bg = QColor{QStringLiteral("#262d3a")};
    p.panel_alt_bg = QColor{QStringLiteral("#28303e")};
    p.status_panel_bg = QColor{QStringLiteral("#262e3b")};
    p.header_bg = QColor{QStringLiteral("#1f2530")};
    p.sub_header_bg = QColor{QStringLiteral("#313a4b")};
    p.toolbar_bg = QColor{QStringLiteral("#2b3342")};
    p.menubar_bg = QColor{QStringLiteral("#252c38")};
    p.filter_bar_bg = QColor{QStringLiteral("#2a3240")};
    p.input_bg = QColor{QStringLiteral("#1b202a")};
    p.input_alt_bg = QColor{QStringLiteral("#1b202a")};
    p.card_bg = QColor{QStringLiteral("#2c3444")};
    p.card_hover_bg = QColor{QStringLiteral("#353f52")};
    p.card_selected_bg = QColor{QStringLiteral("#1e466e")};
    p.card_drop_bg = QColor{QStringLiteral("#1d485c")};
    p.list_bg = QColor{QStringLiteral("#242b38")};
    p.list_hover_bg = QColor{QStringLiteral("#2f3848")};
    p.list_selected_bg = QColor{QStringLiteral("#254a73")};
    p.order_selected_bg = QColor{QStringLiteral("#21446b")};
    p.menu_bg = QColor{QStringLiteral("#293140")};
    p.menu_hover_bg = QColor{QStringLiteral("#364054")};

    p.border_strong = QColor{QStringLiteral("#181d26")};
    p.border_mid = QColor{QStringLiteral("#1c222d")};
    p.border_subtle = QColor{QStringLiteral("#394457")};
    p.border_card = QColor{QStringLiteral("#3a4559")};
    p.border_divider = QColor{QStringLiteral("#30394a")};
    p.border_hover = QColor{QStringLiteral("#54637d")};
    p.scrollbar_track = QColor{QStringLiteral("#1f2530")};
    p.scrollbar_handle = QColor{QStringLiteral("#3f4b61")};
    p.scrollbar_handle_hover = QColor{QStringLiteral("#566580")};

    p.text_primary = QColor{QStringLiteral("#e6ebf4")};
    p.text_secondary = QColor{QStringLiteral("#cfd7e6")};
    p.text_muted = QColor{QStringLiteral("#97a3b8")};
    p.text_subtle = QColor{QStringLiteral("#7a879e")};
    p.text_disabled = QColor{QStringLiteral("#5c677d")};
    p.text_selected_sub = QColor{QStringLiteral("#bae6fd")};
    p.text_link = QColor{QStringLiteral("#7dd3fc")};

    p.button_bg = QColor{QStringLiteral("#323b4c")};
    p.button_alt_bg = QColor{QStringLiteral("#30394a")};
    p.button_border = QColor{QStringLiteral("#424e63")};
    p.button_alt_border = QColor{QStringLiteral("#3f4b60")};
    p.button_hover_bg = QColor{QStringLiteral("#3d475c")};
    p.button_hover_border = QColor{QStringLiteral("#5a6a85")};
    p.button_pressed_bg = QColor{QStringLiteral("#252c38")};
    p.button_disabled_bg = QColor{QStringLiteral("#272f3d")};
    p.button_disabled_border = QColor{QStringLiteral("#323b4c")};
    p.badge_bg = QColor{QStringLiteral("#313a4b")};
    p.badge_border = QColor{QStringLiteral("#414c61")};
    p.badge_fg = QColor{QStringLiteral("#aeb9cc")};
    p.tab_bar_bg = QColor{QStringLiteral("#323b4b")};
    p.tab_inactive_bg = QColor{QStringLiteral("#394356")};
    p.tab_inactive_fg = QColor{QStringLiteral("#cfd7e6")};
    p.rail_bg = QColor{QStringLiteral("#242b37")};
    p.rail_hover_bg = QColor{QStringLiteral("#313a4a")};
    p.rail_border = QColor{QStringLiteral("#323b4b")};

    p.accent = QColor{QStringLiteral("#0273b5")};
    p.accent_hover = QColor{QStringLiteral("#0384cf")};
    p.accent_pressed = QColor{QStringLiteral("#025e94")};
    p.accent_border = QColor{QStringLiteral("#0ea5e9")};
    p.accent_hover_border = QColor{QStringLiteral("#38bdf8")};
    p.tab_active_bg = QColor{QStringLiteral("#0273b5")};
    p.drop_highlight = QColor{QStringLiteral("#7dd3fc")};

    p.live_fg = QColor{QStringLiteral("#34d399")};
    p.live_pill_bg = QColor{52, 211, 153, 40};
    p.live_pill_border = QColor{52, 211, 153, 125};
    p.live_clear_bg = QColor{QStringLiteral("#2e3748")};
    p.live_clear_border = QColor{QStringLiteral("#3e4a60")};

    p.cue_card_bg = QColor{QStringLiteral("#1b212b")};
    p.cue_card_border = QColor{QStringLiteral("#384357")};
    p.thumb_well_bg = QColor{QStringLiteral("#181d26")};
    p.thumb_well_border = QColor{QStringLiteral("#343f52")};
    p.monitor_row_bg = QColor{QStringLiteral("#1b212b")};
    p.preview_well_bg = QColor{QStringLiteral("#12161f")};
    p.preview_black_bg = QColor{QStringLiteral("#090b10")};
    return p;
}

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

ThemePalette make_high_contrast_palette() {
    ThemePalette p = make_dark_palette();
    p.id = ThemeId::high_contrast;
    p.is_dark = true;

    p.window_bg = QColor{QStringLiteral("#000000")};
    p.panel_bg = QColor{QStringLiteral("#000000")};
    p.panel_alt_bg = QColor{QStringLiteral("#050505")};
    p.status_panel_bg = QColor{QStringLiteral("#000000")};
    p.header_bg = QColor{QStringLiteral("#0a0a0a")};
    p.sub_header_bg = QColor{QStringLiteral("#141414")};
    p.toolbar_bg = QColor{QStringLiteral("#0a0a0a")};
    p.menubar_bg = QColor{QStringLiteral("#000000")};
    p.filter_bar_bg = QColor{QStringLiteral("#0a0a0a")};
    p.input_bg = QColor{QStringLiteral("#000000")};
    p.input_alt_bg = QColor{QStringLiteral("#000000")};
    p.card_bg = QColor{QStringLiteral("#0c0c0c")};
    p.card_hover_bg = QColor{QStringLiteral("#1c1c1c")};
    p.card_selected_bg = QColor{QStringLiteral("#003a75")};
    p.card_drop_bg = QColor{QStringLiteral("#004b66")};
    p.list_bg = QColor{QStringLiteral("#000000")};
    p.list_hover_bg = QColor{QStringLiteral("#1a1a1a")};
    p.list_selected_bg = QColor{QStringLiteral("#003a75")};
    p.order_selected_bg = QColor{QStringLiteral("#003a75")};
    p.menu_bg = QColor{QStringLiteral("#000000")};
    p.menu_hover_bg = QColor{QStringLiteral("#005fcc")};

    p.border_strong = QColor{QStringLiteral("#ffffff")};
    p.border_mid = QColor{QStringLiteral("#b0b0b0")};
    p.border_subtle = QColor{QStringLiteral("#8f8f8f")};
    p.border_card = QColor{QStringLiteral("#949494")};
    p.border_divider = QColor{QStringLiteral("#5c5c5c")};
    p.border_hover = QColor{QStringLiteral("#ffd60a")};
    p.scrollbar_track = QColor{QStringLiteral("#000000")};
    p.scrollbar_handle = QColor{QStringLiteral("#7a7a7a")};
    p.scrollbar_handle_hover = QColor{QStringLiteral("#ffffff")};

    p.text_primary = QColor{QStringLiteral("#ffffff")};
    p.text_secondary = QColor{QStringLiteral("#ffffff")};
    p.text_muted = QColor{QStringLiteral("#d8d8d8")};
    p.text_subtle = QColor{QStringLiteral("#bdbdbd")};
    p.text_disabled = QColor{QStringLiteral("#7a7a7a")};
    p.text_selected = QColor{QStringLiteral("#ffffff")};
    p.text_selected_sub = QColor{QStringLiteral("#bae6fd")};
    p.text_on_accent = QColor{QStringLiteral("#ffffff")};
    p.text_link = QColor{QStringLiteral("#66c2ff")};

    p.button_bg = QColor{QStringLiteral("#141414")};
    p.button_alt_bg = QColor{QStringLiteral("#141414")};
    p.button_border = QColor{QStringLiteral("#ffffff")};
    p.button_alt_border = QColor{QStringLiteral("#ffffff")};
    p.button_hover_bg = QColor{QStringLiteral("#262626")};
    p.button_hover_border = QColor{QStringLiteral("#ffd60a")};
    p.button_pressed_bg = QColor{QStringLiteral("#003a75")};
    p.button_disabled_bg = QColor{QStringLiteral("#0a0a0a")};
    p.button_disabled_border = QColor{QStringLiteral("#525252")};
    p.badge_bg = QColor{QStringLiteral("#141414")};
    p.badge_border = QColor{QStringLiteral("#ffffff")};
    p.badge_fg = QColor{QStringLiteral("#ffffff")};
    p.tab_bar_bg = QColor{QStringLiteral("#141414")};
    p.tab_inactive_bg = QColor{QStringLiteral("#000000")};
    p.tab_inactive_fg = QColor{QStringLiteral("#ffffff")};
    p.tab_active_bg = QColor{QStringLiteral("#0066ff")};
    p.rail_bg = QColor{QStringLiteral("#000000")};
    p.rail_hover_bg = QColor{QStringLiteral("#262626")};
    p.rail_border = QColor{QStringLiteral("#8f8f8f")};
    p.rail_icon = QColor{QStringLiteral("#ffffff")};
    p.icon_neutral = QColor{QStringLiteral("#ffffff")};
    p.icon_muted = QColor{QStringLiteral("#e5e5e5")};
    p.icon_dim = QColor{QStringLiteral("#b8b8b8")};

    p.accent = QColor{QStringLiteral("#0066ff")};
    p.accent_hover = QColor{QStringLiteral("#2680ff")};
    p.accent_pressed = QColor{QStringLiteral("#004ecc")};
    p.accent_border = QColor{QStringLiteral("#ffffff")};
    p.accent_hover_border = QColor{QStringLiteral("#ffd60a")};
    p.drop_highlight = QColor{QStringLiteral("#ffd60a")};

    p.live_fg = QColor{QStringLiteral("#00ff88")};
    p.live_pill_bg = QColor{0, 255, 136, 48};
    p.live_pill_border = QColor{0, 255, 136, 220};
    p.live_clear_bg = QColor{QStringLiteral("#141414")};
    p.live_clear_border = QColor{QStringLiteral("#ffffff")};

    p.danger_fg = QColor{QStringLiteral("#ff5c5c")};
    p.danger_soft_fg = QColor{QStringLiteral("#ff8080")};
    p.warning_fg = QColor{QStringLiteral("#ffd60a")};

    p.cue_card_bg = QColor{QStringLiteral("#000000")};
    p.cue_card_border = QColor{QStringLiteral("#b0b0b0")};
    p.thumb_well_bg = QColor{QStringLiteral("#000000")};
    p.thumb_well_border = QColor{QStringLiteral("#ffffff")};
    p.monitor_row_bg = QColor{QStringLiteral("#000000")};
    p.preview_well_bg = QColor{QStringLiteral("#000000")};
    p.preview_well_border = QColor{QStringLiteral("#ffffff")};
    p.preview_well_text = QColor{QStringLiteral("#d4d4d4")};
    p.preview_black_bg = QColor{QStringLiteral("#000000")};
    p.preview_black_text = QColor{QStringLiteral("#d4d4d4")};

    p.reflow_break_bg = QColor{QStringLiteral("#ffd60a")};
    p.reflow_break_fg = QColor{QStringLiteral("#000000")};
    return p;
}

}  // namespace shekinah::ui::theme_internal

