#include "theme_palettes.hpp"

namespace shekinah::ui::theme_internal {

ThemePalette make_light_palette() {
    ThemePalette p;
    p.id = ThemeId::light;
    p.is_dark = false;

    p.window_bg = QColor{QStringLiteral("#eef1f5")};
    p.panel_bg = QColor{QStringLiteral("#f7f8fa")};
    p.panel_alt_bg = QColor{QStringLiteral("#f4f6f9")};
    p.status_panel_bg = QColor{QStringLiteral("#f7f8fa")};
    p.header_bg = QColor{QStringLiteral("#e7ebf1")};
    p.sub_header_bg = QColor{QStringLiteral("#e9edf3")};
    p.toolbar_bg = QColor{QStringLiteral("#f0f2f6")};
    p.menubar_bg = QColor{QStringLiteral("#f5f7fa")};
    p.filter_bar_bg = QColor{QStringLiteral("#edf0f5")};
    p.input_bg = QColor{QStringLiteral("#ffffff")};
    p.input_alt_bg = QColor{QStringLiteral("#ffffff")};
    p.card_bg = QColor{QStringLiteral("#ffffff")};
    p.card_hover_bg = QColor{QStringLiteral("#f0f4fa")};
    p.card_selected_bg = QColor{QStringLiteral("#dbeafe")};
    p.card_drop_bg = QColor{QStringLiteral("#d6f0ff")};
    p.list_bg = QColor{QStringLiteral("#ffffff")};
    p.list_hover_bg = QColor{QStringLiteral("#eef3fa")};
    p.list_selected_bg = QColor{QStringLiteral("#dbeafe")};
    p.order_selected_bg = QColor{QStringLiteral("#dbeafe")};
    p.menu_bg = QColor{QStringLiteral("#ffffff")};
    p.menu_hover_bg = QColor{QStringLiteral("#e5ebf3")};

    p.border_strong = QColor{QStringLiteral("#cbd3df")};
    p.border_mid = QColor{QStringLiteral("#d5dce6")};
    p.border_subtle = QColor{QStringLiteral("#d8dfe9")};
    p.border_card = QColor{QStringLiteral("#d4dbe6")};
    p.border_divider = QColor{QStringLiteral("#e6eaf1")};
    p.border_hover = QColor{QStringLiteral("#94a1b4")};
    p.scrollbar_track = QColor{QStringLiteral("#eef1f5")};
    p.scrollbar_handle = QColor{QStringLiteral("#c0c8d6")};
    p.scrollbar_handle_hover = QColor{QStringLiteral("#98a3b5")};

    p.text_primary = QColor{QStringLiteral("#171a21")};
    p.text_secondary = QColor{QStringLiteral("#2c3340")};
    p.text_muted = QColor{QStringLiteral("#566070")};
    p.text_subtle = QColor{QStringLiteral("#747e8f")};
    p.text_disabled = QColor{QStringLiteral("#a2aab8")};
    p.text_selected = QColor{QStringLiteral("#0f172a")};
    p.text_selected_sub = QColor{QStringLiteral("#1d4ed8")};
    p.text_on_accent = QColor{QStringLiteral("#ffffff")};
    p.text_link = QColor{QStringLiteral("#0066cc")};
    p.text_link_hover = QColor{QStringLiteral("#004999")};

    p.button_bg = QColor{QStringLiteral("#ffffff")};
    p.button_alt_bg = QColor{QStringLiteral("#ffffff")};
    p.button_border = QColor{QStringLiteral("#cbd2dd")};
    p.button_alt_border = QColor{QStringLiteral("#cbd2dd")};
    p.button_hover_bg = QColor{QStringLiteral("#edf1f7")};
    p.button_hover_border = QColor{QStringLiteral("#96a3b5")};
    p.button_pressed_bg = QColor{QStringLiteral("#e0e6f0")};
    p.button_disabled_bg = QColor{QStringLiteral("#f1f3f6")};
    p.button_disabled_border = QColor{QStringLiteral("#e0e4ec")};
    p.badge_bg = QColor{QStringLiteral("#e1e5ee")};
    p.badge_border = QColor{QStringLiteral("#cbd2dd")};
    p.badge_fg = QColor{QStringLiteral("#464f60")};
    p.tab_bar_bg = QColor{QStringLiteral("#e4e8ef")};
    p.tab_inactive_bg = QColor{QStringLiteral("#eef1f6")};
    p.tab_inactive_fg = QColor{QStringLiteral("#3c4452")};
    p.tab_active_bg = QColor{QStringLiteral("#006ee6")};
    p.tab_active_fg = QColor{QStringLiteral("#ffffff")};
    p.rail_bg = QColor{QStringLiteral("#eceff5")};
    p.rail_hover_bg = QColor{QStringLiteral("#dde3ee")};
    p.rail_border = QColor{QStringLiteral("#d4dbe6")};
    p.rail_icon = QColor{QStringLiteral("#3a4250")};
    p.icon_neutral = QColor{QStringLiteral("#3a4250")};
    p.icon_muted = QColor{QStringLiteral("#566070")};
    p.icon_dim = QColor{QStringLiteral("#747e8f")};
    p.icon_empty = QColor{QStringLiteral("#98a2b3")};
    p.quick_clear_bg = QColor{QStringLiteral("#1f2633")};
    p.quick_clear_fg = QColor{QStringLiteral("#ffffff")};
    p.quick_clear_border = QColor{QStringLiteral("#131720")};
    p.quick_clear_hover_bg = QColor{QStringLiteral("#323c4f")};
    p.quick_clear_pressed_bg = QColor{QStringLiteral("#10141c")};

    p.accent = QColor{QStringLiteral("#006ee6")};
    p.accent_hover = QColor{QStringLiteral("#005ec4")};
    p.accent_pressed = QColor{QStringLiteral("#004da3")};
    p.accent_border = QColor{QStringLiteral("#005ec4")};
    p.accent_hover_border = QColor{QStringLiteral("#004da3")};
    p.drop_highlight = QColor{QStringLiteral("#0284c7")};

    p.live_fg = QColor{QStringLiteral("#0a8750")};
    p.live_pill_bg = QColor{10, 135, 80, 32};
    p.live_pill_border = QColor{10, 135, 80, 125};
    p.live_clear_bg = QColor{QStringLiteral("#e5e9f0")};
    p.live_clear_border = QColor{QStringLiteral("#c7d0dd")};

    p.danger_fg = QColor{QStringLiteral("#c92a2a")};
    p.danger_soft_fg = QColor{QStringLiteral("#b91c1c")};
    p.danger_icon = QColor{QStringLiteral("#b91c1c")};
    p.danger_bg = QColor{QStringLiteral("#dc2626")};
    p.danger_hover_bg = QColor{QStringLiteral("#fee2e2")};
    p.danger_border = QColor{QStringLiteral("#b91c1c")};
    p.danger_hover_border = QColor{QStringLiteral("#ef4444")};
    p.danger_pill_bg = QColor{220, 38, 38, 30};
    p.danger_pill_border = QColor{220, 38, 38, 125};
    p.danger_text_on_bg = QColor{QStringLiteral("#991b1b")};

    p.warning_fg = QColor{QStringLiteral("#d97706")};

    p.song_badge_fg = QColor{QStringLiteral("#0284c7")};
    p.song_badge_bg = QColor{2, 132, 199, 28};
    p.song_badge_border = QColor{2, 132, 199, 85};
    p.sermon_badge_fg = QColor{QStringLiteral("#b45309")};
    p.sermon_badge_bg = QColor{217, 119, 6, 28};
    p.sermon_badge_border = QColor{217, 119, 6, 85};
    p.video_badge_fg = QColor{QStringLiteral("#ddd6fe")};
    p.video_badge_bg = QColor{46, 16, 101, 215};
    p.video_badge_border = QColor{167, 139, 250, 140};
    p.image_badge_fg = QColor{QStringLiteral("#a7f3d0")};
    p.image_badge_bg = QColor{6, 78, 59, 215};
    p.image_badge_border = QColor{52, 211, 153, 140};
    p.size_pill_bg = QColor{20, 24, 33, 200};
    p.size_pill_border = QColor{71, 85, 105, 150};
    p.size_pill_fg = QColor{QStringLiteral("#e2e8f0")};
    p.add_pill_bg = QColor{230, 236, 245, 235};
    p.add_pill_border = QColor{148, 163, 184, 200};
    p.add_pill_fg = QColor{QStringLiteral("#1e293b")};
    p.add_pill_selected_bg = QColor{0, 110, 230, 42};
    p.add_pill_selected_border = QColor{0, 110, 230, 140};
    p.add_pill_selected_fg = QColor{QStringLiteral("#004da3")};

    p.cue_card_bg = QColor{QStringLiteral("#dfe4ec")};
    p.cue_card_border = QColor{QStringLiteral("#c2cad6")};
    p.thumb_well_bg = QColor{QStringLiteral("#1c2029")};
    p.thumb_well_border = QColor{QStringLiteral("#c6ceda")};
    p.monitor_row_bg = QColor{QStringLiteral("#e2e6ed")};
    p.preview_well_bg = QColor{QStringLiteral("#12161f")};
    p.preview_well_border = QColor{QStringLiteral("#30384a")};
    p.preview_well_text = QColor{QStringLiteral("#7b8499")};
    p.preview_black_bg = QColor{QStringLiteral("#08090c")};
    p.preview_black_text = QColor{QStringLiteral("#646c7d")};

    p.reflow_break_bg = QColor{QStringLiteral("#fde047")};
    p.reflow_break_fg = QColor{QStringLiteral("#1e293b")};
    return p;
}


}  // namespace shekinah::ui::theme_internal

