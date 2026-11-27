#pragma once

#include <QColor>
#include <QString>

namespace shekinah::ui {

enum class ThemeId {
    system,
    dark,
    light,
    midnight,
    slate,
    warm_light,
    high_contrast,
};

struct ThemeInfo {
    ThemeId id{ThemeId::dark};
    QString key;
    QString label;
    QString description;
    bool is_dark{true};
};

struct ThemePalette {
    ThemeId id{ThemeId::dark};
    bool is_dark{true};

    // Surfaces
    QColor window_bg;
    QColor panel_bg;
    QColor panel_alt_bg;
    QColor status_panel_bg;
    QColor header_bg;
    QColor sub_header_bg;
    QColor toolbar_bg;
    QColor menubar_bg;
    QColor filter_bar_bg;
    QColor input_bg;
    QColor input_alt_bg;
    QColor card_bg;
    QColor card_hover_bg;
    QColor card_selected_bg;
    QColor card_drop_bg;
    QColor list_bg;
    QColor list_hover_bg;
    QColor list_selected_bg;
    QColor order_selected_bg;
    QColor menu_bg;
    QColor menu_hover_bg;

    // Borders & Scrollbars
    QColor border_strong;
    QColor border_mid;
    QColor border_subtle;
    QColor border_card;
    QColor border_divider;
    QColor border_hover;
    QColor scrollbar_track;
    QColor scrollbar_handle;
    QColor scrollbar_handle_hover;

    // Typography
    QColor text_primary;
    QColor text_secondary;
    QColor text_muted;
    QColor text_subtle;
    QColor text_disabled;
    QColor text_selected;
    QColor text_selected_sub;
    QColor text_on_accent;
    QColor text_link;
    QColor text_link_hover;

    // Interactive Controls
    QColor button_bg;
    QColor button_alt_bg;
    QColor button_border;
    QColor button_alt_border;
    QColor button_hover_bg;
    QColor button_hover_border;
    QColor button_pressed_bg;
    QColor button_disabled_bg;
    QColor button_disabled_border;
    QColor badge_bg;
    QColor badge_border;
    QColor badge_fg;
    QColor tab_bar_bg;
    QColor tab_inactive_bg;
    QColor tab_inactive_fg;
    QColor tab_active_bg;
    QColor tab_active_fg;
    QColor rail_bg;
    QColor rail_hover_bg;
    QColor rail_border;
    QColor rail_icon;
    QColor icon_neutral;
    QColor icon_muted;
    QColor icon_dim;
    QColor icon_empty;
    QColor quick_clear_bg;
    QColor quick_clear_fg;
    QColor quick_clear_border;
    QColor quick_clear_hover_bg;
    QColor quick_clear_pressed_bg;

    // Accent & Semantic Status
    QColor accent;
    QColor accent_hover;
    QColor accent_pressed;
    QColor accent_border;
    QColor accent_hover_border;
    QColor drop_highlight;

    QColor live_fg;
    QColor live_pill_bg;
    QColor live_pill_border;
    QColor live_clear_bg;
    QColor live_clear_border;

    QColor danger_fg;
    QColor danger_soft_fg;
    QColor danger_icon;
    QColor danger_bg;
    QColor danger_hover_bg;
    QColor danger_border;
    QColor danger_hover_border;
    QColor danger_pill_bg;
    QColor danger_pill_border;
    QColor danger_text_on_bg;

    QColor warning_fg;

    // Domain Badges & Pills
    QColor song_badge_fg;
    QColor song_badge_bg;
    QColor song_badge_border;
    QColor sermon_badge_fg;
    QColor sermon_badge_bg;
    QColor sermon_badge_border;
    QColor video_badge_fg;
    QColor video_badge_bg;
    QColor video_badge_border;
    QColor image_badge_fg;
    QColor image_badge_bg;
    QColor image_badge_border;
    QColor size_pill_bg;
    QColor size_pill_border;
    QColor size_pill_fg;
    QColor add_pill_bg;
    QColor add_pill_border;
    QColor add_pill_fg;
    QColor add_pill_selected_bg;
    QColor add_pill_selected_border;
    QColor add_pill_selected_fg;

    // Cue Card & Stage Preview
    QColor cue_card_bg;
    QColor cue_card_border;
    QColor thumb_well_bg;
    QColor thumb_well_border;
    QColor monitor_row_bg;
    QColor preview_well_bg;
    QColor preview_well_border;
    QColor preview_well_text;
    QColor preview_black_bg;
    QColor preview_black_text;

    // Reflow Editor Break Marker
    QColor reflow_break_bg;
    QColor reflow_break_fg;
};

}  // namespace shekinah::ui
