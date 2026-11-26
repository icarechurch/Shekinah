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

}  // namespace shekinah::ui::theme_internal
