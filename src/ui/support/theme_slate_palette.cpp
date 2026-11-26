#include "theme_palettes.hpp"

namespace shekinah::ui::theme_internal {

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

}  // namespace shekinah::ui::theme_internal
