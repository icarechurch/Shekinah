#include "shekinah/ui/support/theme.hpp"

#include <QApplication>
#include <QGuiApplication>
#include <QSettings>
#include <QStyleHints>

namespace shekinah::ui {
namespace {

constexpr auto kThemeSettingsKey = "ui/theme";

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

}  // namespace

ThemeManager::ThemeManager() {
    apply_resolved_theme(false);
    if (QGuiApplication::instance() != nullptr && QGuiApplication::styleHints() != nullptr) {
        QObject::connect(QGuiApplication::styleHints(),
                         &QStyleHints::colorSchemeChanged,
                         this,
                         [this](Qt::ColorScheme) {
                             if (current_id_ == ThemeId::system) {
                                 apply_resolved_theme(true);
                             }
                         });
    }
}

ThemeManager& ThemeManager::instance() {
    static ThemeManager manager;
    return manager;
}

void ThemeManager::set_theme(ThemeId id) {
    const ThemeId next_resolved = resolve_theme_id(id);
    if (current_id_ == id && resolved_id_ == next_resolved) {
        return;
    }
    current_id_ = id;
    apply_resolved_theme(true);
}

bool ThemeManager::set_theme_by_key(const QString& key) {
    const QString normalized = key.trimmed().toLower();
    for (const auto& info : available_themes()) {
        if (info.key == normalized) {
            set_theme(info.id);
            return true;
        }
    }
    return false;
}

void ThemeManager::load_from_settings(const QSettings& settings) {
    const QString key = settings.value(QString::fromLatin1(kThemeSettingsKey),
                                       QStringLiteral("dark"))
                            .toString();
    if (!set_theme_by_key(key)) {
        set_theme(ThemeId::dark);
    }
}

void ThemeManager::save_to_settings(QSettings& settings) const {
    settings.setValue(QString::fromLatin1(kThemeSettingsKey), info_for(current_id_).key);
}

const std::vector<ThemeInfo>& ThemeManager::available_themes() {
    static const std::vector<ThemeInfo> themes = {
        {ThemeId::system,
         QStringLiteral("system"),
         QStringLiteral("System (Auto)"),
         QStringLiteral("Follow the operating system light or dark appearance"),
         false},
        {ThemeId::dark,
         QStringLiteral("dark"),
         QStringLiteral("Dark"),
         QStringLiteral("Charcoal studio operator theme"),
         true},
        {ThemeId::light,
         QStringLiteral("light"),
         QStringLiteral("Light"),
         QStringLiteral("Crisp daylight workspace theme"),
         false},
        {ThemeId::midnight,
         QStringLiteral("midnight"),
         QStringLiteral("Midnight"),
         QStringLiteral("Ultra-low-glare theme for darkened booths"),
         true},
        {ThemeId::slate,
         QStringLiteral("slate"),
         QStringLiteral("Slate"),
         QStringLiteral("Cool Nordic blue-grey studio theme"),
         true},
        {ThemeId::warm_light,
         QStringLiteral("warm-light"),
         QStringLiteral("Warm Light"),
         QStringLiteral("Soft warm paper daylight theme"),
         false},
        {ThemeId::high_contrast,
         QStringLiteral("high-contrast"),
         QStringLiteral("High Contrast"),
         QStringLiteral("Accessible high-contrast dark theme"),
         true},
    };
    return themes;
}

ThemeInfo ThemeManager::info_for(ThemeId id) {
    for (const auto& info : available_themes()) {
        if (info.id == id) {
            return info;
        }
    }
    return available_themes().at(1);
}

ThemeId ThemeManager::resolve_theme_id(ThemeId id) {
    if (id != ThemeId::system) {
        return id;
    }
    if (QGuiApplication::instance() != nullptr && QGuiApplication::styleHints() != nullptr &&
        QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Light) {
        return ThemeId::light;
    }
    return ThemeId::dark;
}

const ThemePalette& ThemeManager::palette_for(ThemeId id) {
    static const ThemePalette kDark = make_dark_palette();
    static const ThemePalette kLight = make_light_palette();
    static const ThemePalette kMidnight = make_midnight_palette();
    static const ThemePalette kSlate = make_slate_palette();
    static const ThemePalette kWarmLight = make_warm_light_palette();
    static const ThemePalette kHighContrast = make_high_contrast_palette();

    switch (resolve_theme_id(id)) {
    case ThemeId::light:
        return kLight;
    case ThemeId::midnight:
        return kMidnight;
    case ThemeId::slate:
        return kSlate;
    case ThemeId::warm_light:
        return kWarmLight;
    case ThemeId::high_contrast:
        return kHighContrast;
    case ThemeId::system:
    case ThemeId::dark:
        return kDark;
    }
    return kDark;
}

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

QString ThemeManager::dialog_stylesheet(const ThemePalette& p) {
    return QStringLiteral(
               "QDialog {"
               "  background-color: %1;"
               "  color: %2;"
               "}"
               "QLabel {"
               "  color: %2;"
               "  background: transparent;"
               "}"
               "QLineEdit, QPlainTextEdit, QTextEdit {"
               "  background-color: %3;"
               "  color: %2;"
               "  border: 1px solid %4;"
               "  border-radius: 4px;"
               "  padding: 4px 8px;"
               "  selection-background-color: %5;"
               "  selection-color: %6;"
               "}"
               "QLineEdit:focus, QPlainTextEdit:focus, QTextEdit:focus, QComboBox:focus {"
               "  border-color: %5;"
               "}"
               "QComboBox {"
               "  background-color: %7;"
               "  color: %2;"
               "  border: 1px solid %4;"
               "  border-radius: 4px;"
               "  padding: 4px 8px;"
               "}"
               "QComboBox QAbstractItemView {"
               "  background-color: %8;"
               "  color: %2;"
               "  border: 1px solid %4;"
               "  selection-background-color: %5;"
               "  selection-color: %6;"
               "}"
               "QListWidget {"
               "  background-color: %9;"
               "  color: %2;"
               "  border: 1px solid %4;"
               "  border-radius: 4px;"
               "  outline: 0;"
               "}"
               "QListWidget::item {"
               "  padding: 5px 8px;"
               "}"
               "QListWidget::item:selected {"
               "  background-color: %10;"
               "  color: %11;"
               "}"
               "QListWidget::item:hover {"
               "  background-color: %12;"
               "}"
               "QPushButton {"
               "  background-color: %7;"
               "  border: 1px solid %13;"
               "  border-radius: 4px;"
               "  color: %2;"
               "  padding: 5px 12px;"
               "  font-weight: 600;"
               "}"
               "QPushButton:hover {"
               "  background-color: %14;"
               "  border-color: %15;"
               "}"
               "QPushButton:pressed {"
               "  background-color: %16;"
               "}"
               "QPushButton:disabled {"
               "  background-color: %17;"
               "  border-color: %18;"
               "  color: %19;"
               "}"
               "QSplitter::handle {"
               "  background-color: %20;"
               "}"
               "QSplitter::handle:hover {"
               "  background-color: %5;"
               "}")
        .arg(p.panel_bg.name(),               // %1
             p.text_primary.name(),           // %2
             p.input_bg.name(),               // %3
             p.border_subtle.name(),          // %4
             p.accent.name(),                 // %5
             p.text_on_accent.name(),         // %6
             p.button_bg.name(),              // %7
             p.menu_bg.name(),                // %8
             p.list_bg.name(),                // %9
             p.list_selected_bg.name(),       // %10
             p.text_selected.name(),          // %11
             p.list_hover_bg.name(),          // %12
             p.button_border.name(),          // %13
             p.button_hover_bg.name(),        // %14
             p.button_hover_border.name(),    // %15
             p.button_pressed_bg.name(),      // %16
             p.button_disabled_bg.name(),     // %17
             p.button_disabled_border.name(), // %18
             p.text_disabled.name(),          // %19
             p.border_strong.name());         // %20
}

void ThemeManager::apply_resolved_theme(bool emit_signal) {
    resolved_id_ = resolve_theme_id(current_id_);
    current_palette_ = palette_for(resolved_id_);
    if (auto* app = qobject_cast<QApplication*>(QCoreApplication::instance()); app != nullptr) {
        app->setPalette(build_qpalette(current_palette_));
    }
    if (emit_signal) {
        emit theme_changed(current_id_);
    }
}

}  // namespace shekinah::ui
