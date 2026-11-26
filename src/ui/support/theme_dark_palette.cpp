#include "theme_palettes.hpp"

namespace shekinah::ui::theme_internal {

ThemePalette make_dark_palette() {
    ThemePalette p;
    p.id = ThemeId::dark;
    p.is_dark = true;

    p.window_bg = QColor{QStringLiteral("#18191d")};
    p.panel_bg = QColor{QStringLiteral("#1b1c21")};
    p.panel_alt_bg = QColor{QStringLiteral("#1d1e23")};
    p.status_panel_bg = QColor{QStringLiteral("#1e1f24")};
    p.header_bg = QColor{QStringLiteral("#191a1e")};
    p.sub_header_bg = QColor{QStringLiteral("#2b2d33")};
    p.toolbar_bg = QColor{QStringLiteral("#202228")};
    p.menubar_bg = QColor{QStringLiteral("#202127")};
    p.filter_bar_bg = QColor{QStringLiteral("#23252a")};
    p.input_bg = QColor{QStringLiteral("#15161a")};
    p.input_alt_bg = QColor{QStringLiteral("#16171a")};
    p.card_bg = QColor{QStringLiteral("#202229")};
    p.card_hover_bg = QColor{QStringLiteral("#262932")};
    p.card_selected_bg = QColor{QStringLiteral("#1b314b")};
    p.card_drop_bg = QColor{QStringLiteral("#183244")};
    p.list_bg = QColor{QStringLiteral("#1a1b1f")};
    p.list_hover_bg = QColor{QStringLiteral("#24262c")};
    p.list_selected_bg = QColor{QStringLiteral("#27446b")};
    p.order_selected_bg = QColor{QStringLiteral("#1d324d")};
    p.menu_bg = QColor{QStringLiteral("#22242a")};
    p.menu_hover_bg = QColor{QStringLiteral("#2d3038")};

    p.border_strong = QColor{QStringLiteral("#111215")};
    p.border_mid = QColor{QStringLiteral("#131417")};
    p.border_subtle = QColor{QStringLiteral("#2c2f38")};
    p.border_card = QColor{QStringLiteral("#2c2f38")};
    p.border_divider = QColor{QStringLiteral("#24262e")};
    p.border_hover = QColor{QStringLiteral("#464b58")};
    p.scrollbar_track = QColor{QStringLiteral("#17181c")};
    p.scrollbar_handle = QColor{QStringLiteral("#353842")};
    p.scrollbar_handle_hover = QColor{QStringLiteral("#4b4f5c")};

    p.text_primary = QColor{QStringLiteral("#e6e8ec")};
    p.text_secondary = QColor{QStringLiteral("#d7dae0")};
    p.text_muted = QColor{QStringLiteral("#8e929b")};
    p.text_subtle = QColor{QStringLiteral("#777c87")};
    p.text_disabled = QColor{QStringLiteral("#555965")};
    p.text_selected = QColor{QStringLiteral("#ffffff")};
    p.text_selected_sub = QColor{QStringLiteral("#a7c7f5")};
    p.text_on_accent = QColor{QStringLiteral("#ffffff")};
    p.text_link = QColor{QStringLiteral("#8ec5ff")};
    p.text_link_hover = QColor{QStringLiteral("#ffffff")};

    p.button_bg = QColor{QStringLiteral("#292b32")};
    p.button_alt_bg = QColor{QStringLiteral("#272930")};
    p.button_border = QColor{QStringLiteral("#383b44")};
    p.button_alt_border = QColor{QStringLiteral("#353842")};
    p.button_hover_bg = QColor{QStringLiteral("#343741")};
    p.button_hover_border = QColor{QStringLiteral("#4c515e")};
    p.button_pressed_bg = QColor{QStringLiteral("#1f2127")};
    p.button_disabled_bg = QColor{QStringLiteral("#1f2026")};
    p.button_disabled_border = QColor{QStringLiteral("#272930")};
    p.badge_bg = QColor{QStringLiteral("#25272e")};
    p.badge_border = QColor{QStringLiteral("#30333d")};
    p.badge_fg = QColor{QStringLiteral("#9ea3ae")};
    p.tab_bar_bg = QColor{QStringLiteral("#292b31")};
    p.tab_inactive_bg = QColor{QStringLiteral("#2f3138")};
    p.tab_inactive_fg = QColor{QStringLiteral("#c5c9d2")};
    p.tab_active_bg = QColor{QStringLiteral("#0a84ff")};
    p.tab_active_fg = QColor{QStringLiteral("#ffffff")};
    p.rail_bg = QColor{QStringLiteral("#1b1c20")};
    p.rail_hover_bg = QColor{QStringLiteral("#282a31")};
    p.rail_border = QColor{QStringLiteral("#262830")};
    p.rail_icon = QColor{QStringLiteral("#c5c9d2")};
    p.icon_neutral = QColor{QStringLiteral("#cfd3dc")};
    p.icon_muted = QColor{QStringLiteral("#9ea3ae")};
    p.icon_dim = QColor{QStringLiteral("#6d727d")};
    p.icon_empty = QColor{QStringLiteral("#4f5460")};
    p.quick_clear_bg = QColor{QStringLiteral("#e6e8ec")};
    p.quick_clear_fg = QColor{QStringLiteral("#141518")};
    p.quick_clear_border = QColor{QStringLiteral("#141518")};
    p.quick_clear_hover_bg = QColor{QStringLiteral("#ffffff")};
    p.quick_clear_pressed_bg = QColor{QStringLiteral("#bfc4cd")};

    p.accent = QColor{QStringLiteral("#0a84ff")};
    p.accent_hover = QColor{QStringLiteral("#2291ff")};
    p.accent_pressed = QColor{QStringLiteral("#066fd9")};
    p.accent_border = QColor{QStringLiteral("#2693ff")};
    p.accent_hover_border = QColor{QStringLiteral("#4ca6ff")};
    p.drop_highlight = QColor{QStringLiteral("#65d6ff")};

    p.live_fg = QColor{QStringLiteral("#22e58a")};
    p.live_pill_bg = QColor{34, 229, 138, 42};
    p.live_pill_border = QColor{34, 229, 138, 120};
    p.live_clear_bg = QColor{QStringLiteral("#262830")};
    p.live_clear_border = QColor{QStringLiteral("#353844")};

    p.danger_fg = QColor{QStringLiteral("#ff5a5f")};
    p.danger_soft_fg = QColor{QStringLiteral("#ff6b6d")};
    p.danger_icon = QColor{QStringLiteral("#d98b8e")};
    p.danger_bg = QColor{QStringLiteral("#7d2326")};
    p.danger_hover_bg = QColor{QStringLiteral("#5b2226")};
    p.danger_border = QColor{QStringLiteral("#d9383a")};
    p.danger_hover_border = QColor{QStringLiteral("#9f3338")};
    p.danger_pill_bg = QColor{217, 56, 58, 56};
    p.danger_pill_border = QColor{217, 56, 58, 140};
    p.danger_text_on_bg = QColor{QStringLiteral("#ffffff")};

    p.warning_fg = QColor{QStringLiteral("#f5a623")};

    p.song_badge_fg = QColor{QStringLiteral("#38bdf8")};
    p.song_badge_bg = QColor{14, 165, 233, 34};
    p.song_badge_border = QColor{56, 189, 248, 75};
    p.sermon_badge_fg = QColor{QStringLiteral("#fbbf24")};
    p.sermon_badge_bg = QColor{245, 158, 11, 34};
    p.sermon_badge_border = QColor{251, 191, 36, 75};
    p.video_badge_fg = QColor{QStringLiteral("#c4b5fd")};
    p.video_badge_bg = QColor{28, 20, 44, 210};
    p.video_badge_border = QColor{167, 139, 250, 110};
    p.image_badge_fg = QColor{QStringLiteral("#6ee7b7")};
    p.image_badge_bg = QColor{10, 28, 22, 210};
    p.image_badge_border = QColor{52, 211, 153, 110};
    p.size_pill_bg = QColor{15, 16, 20, 200};
    p.size_pill_border = QColor{60, 65, 78, 150};
    p.size_pill_fg = QColor{QStringLiteral("#cbd0da")};
    p.add_pill_bg = QColor{45, 49, 60, 210};
    p.add_pill_border = QColor{76, 82, 98, 180};
    p.add_pill_fg = QColor{QStringLiteral("#cfd4de")};
    p.add_pill_selected_bg = QColor{10, 132, 255, 65};
    p.add_pill_selected_border = QColor{101, 214, 255, 130};
    p.add_pill_selected_fg = QColor{QStringLiteral("#dff0ff")};

    p.cue_card_bg = QColor{QStringLiteral("#121212")};
    p.cue_card_border = QColor{QStringLiteral("#303030")};
    p.thumb_well_bg = QColor{QStringLiteral("#111216")};
    p.thumb_well_border = QColor{QStringLiteral("#292c36")};
    p.monitor_row_bg = QColor{QStringLiteral("#121316")};
    p.preview_well_bg = QColor{QStringLiteral("#08090c")};
    p.preview_well_border = QColor{QStringLiteral("#23262e")};
    p.preview_well_text = QColor{QStringLiteral("#424652")};
    p.preview_black_bg = QColor{QStringLiteral("#040506")};
    p.preview_black_text = QColor{QStringLiteral("#3f434d")};

    p.reflow_break_bg = QColor{QStringLiteral("#f4c95d")};
    p.reflow_break_fg = QColor{QStringLiteral("#18191d")};
    return p;
}


}  // namespace shekinah::ui::theme_internal

