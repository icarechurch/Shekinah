#include "theme_palettes.hpp"

namespace shekinah::ui::theme_internal {

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
