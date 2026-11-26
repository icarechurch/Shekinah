#include "shekinah/ui/program_status_widget.hpp"
#include "program_status_components.hpp"

#include "shekinah/media/slide_renderer.hpp"
#include "shekinah/ui/cue_grid_widget.hpp"
#include "shekinah/ui/support/elided_label.hpp"
#include "shekinah/ui/support/theme.hpp"

#include <QAbstractItemView>
#include <QFrame>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMenu>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QProgressBar>
#include <QPushButton>
#include <QScreen>
#include <QScrollArea>
#include <QStackedWidget>
#include <QVBoxLayout>

#include <algorithm>
#include <cmath>

namespace shekinah::ui {
void ProgramStatusWidget::apply_theme() {
    const auto& p = ThemeManager::instance().palette();
    setStyleSheet(
        QStringLiteral(
            "#program-status {"
            "  background-color: %1;"
            "  border-left: 1px solid %2;"
            "  color: %3;"
            "}"
            "#program-header-bar {"
            "  background-color: %4;"
            "  border-bottom: 1px solid %2;"
            "}"
            "#program-status-title {"
            "  color: %5;"
            "  font-size: 10px;"
            "  font-weight: 700;"
            "  letter-spacing: 1.1px;"
            "  border: none;"
            "  background: transparent;"
            "}"
            "#monitor-row {"
            "  background-color: %6;"
            "}"
            "#clear-gutter { background: transparent; }"
            "QPushButton#quick-clear-all {"
            "  background-color: %7;"
            "  border: 1px solid %8;"
            "  border-radius: 10px;"
            "}"
            "QPushButton#quick-clear-all:hover { background-color: %9; }"
            "QPushButton#quick-clear-all:pressed { background-color: %10; }"
            "#program-action-rail {"
            "  background-color: %11;"
            "  border-left: 1px solid %2;"
            "}"
            "#program-status QPushButton[rail=\"true\"] {"
            "  background-color: %11;"
            "  border: none;"
            "  border-bottom: 1px solid %12;"
            "  border-radius: 0px;"
            "  padding: 0px;"
            "}"
            "#program-status QPushButton[rail=\"true\"]:hover {"
            "  background-color: %13;"
            "}"
            "#program-status QPushButton[rail=\"true\"]:checked {"
            "  background-color: %14;"
            "}"
            "#screen-selector-bar {"
            "  background-color: %15;"
            "  border-top: 1px solid %16;"
            "  border-bottom: 1px solid %16;"
            "}"
            "#output-target {"
            "  color: %17;"
            "  font-size: 11px;"
            "  font-weight: 600;"
            "  border: none;"
            "  background: transparent;"
            "}"
            "#audience-mode {"
            "  color: %5;"
            "  font-size: 10px;"
            "  border: none;"
            "  background: transparent;"
            "}"
            "#screen-menu-button {"
            "  background: transparent;"
            "  border: none;"
            "  border-radius: 3px;"
            "}"
            "#screen-menu-button:hover { background: %18; }"
            "#program-status-strip { background-color: %19; }"
            "#program-status QPushButton[statusIndicator=\"true\"] {"
            "  background: transparent;"
            "  border: none;"
            "  border-radius: 3px;"
            "}"
            "#program-status QPushButton[statusIndicator=\"true\"]:hover {"
            "  background: %18;"
            "}"
            "#slide-progress {"
            "  background-color: %20;"
            "  border: none;"
            "}"
            "#slide-progress::chunk { background-color: %14; }"
            "#global-controls {"
            "  background-color: %19;"
            "  border-bottom: 1px solid %2;"
            "}"
            "#program-status QWidget#global-controls QPushButton {"
            "  background-color: %21;"
            "  border: 1px solid %22;"
            "  border-radius: 4px;"
            "  color: %3;"
            "  font-size: 10px;"
            "  font-weight: 600;"
            "  padding: 4px 4px;"
            "  min-height: 24px;"
            "}"
            "#program-status QWidget#global-controls QPushButton:hover {"
            "  background-color: %23;"
            "  border-color: %24;"
            "}"
            "#program-status QWidget#global-controls QPushButton:pressed {"
            "  background-color: %25;"
            "}"
            "#drawer-tab-bar {"
            "  background-color: %26;"
            "  border-bottom: 1px solid %2;"
            "}"
            "#drawer-page-live-scroll, #drawer-page-live,"
            "#drawer-page-screens-scroll, #drawer-page-screens,"
            "#drawer-page-cues {"
            "  background-color: %27;"
            "}"
            "#program-status QWidget[sectionHeader=\"true\"] {"
            "  background-color: %28;"
            "  border-top: 1px solid %16;"
            "  border-bottom: 1px solid %16;"
            "}"
            "#program-status QLabel[sectionTitle=\"true\"] {"
            "  color: %17;"
            "  font-size: 10px;"
            "  font-weight: 700;"
            "  letter-spacing: 0.8px;"
            "  border: none;"
            "  background: transparent;"
            "}"
            "#program-status QPushButton[linkAction=\"true\"] {"
            "  background: transparent;"
            "  border: none;"
            "  color: %29;"
            "  font-size: 10px;"
            "  font-weight: 600;"
            "  padding: 2px 4px;"
            "}"
            "#program-status QPushButton[linkAction=\"true\"]:hover {"
            "  color: %30;"
            "}"
            "#live-card, #memory-panel {"
            "  background-color: %4;"
            "}"
            "#next-up-card {"
            "  background-color: %31;"
            "}"
            "#screen-actions-bar {"
            "  background-color: %32;"
            "}"
            "#live-item-title {"
            "  color: %3;"
            "  font-size: 11px;"
            "  font-weight: 600;"
            "  background: transparent;"
            "}"
            "#live-slide-counter, #live-asset-label {"
            "  color: %5;"
            "  font-size: 10px;"
            "  font-weight: 600;"
            "  background: transparent;"
            "}"
            "#live-lyrics-preview {"
            "  color: %17;"
            "  font-size: 11px;"
            "  background-color: %20;"
            "  border: 1px solid %33;"
            "  border-radius: 4px;"
            "  padding: 6px 8px;"
            "}"
            "#live-set-bg-button {"
            "  background: %21;"
            "  border: 1px solid %22;"
            "  border-radius: 3px;"
            "  color: %17;"
            "  font-size: 10px;"
            "  padding: 2px 7px;"
            "}"
            "#live-set-bg-button:hover { background: %23; }"
            "#next-up-header-button {"
            "  background: transparent;"
            "  border: none;"
            "  padding: 2px;"
            "}"
            "#next-up-header-button:hover {"
            "  background: %18;"
            "  border-radius: 3px;"
            "}"
            "#next-summary-label {"
            "  color: %5;"
            "  font-size: 11px;"
            "  background: transparent;"
            "}"
            "#next-up-action-button {"
            "  background-color: %21;"
            "  border: 1px solid %22;"
            "  border-radius: 5px;"
            "  color: %3;"
            "  font-size: 11px;"
            "  font-weight: 600;"
            "  padding: 6px 10px;"
            "}"
            "#next-up-action-button:hover {"
            "  background-color: %23;"
            "  border-color: %14;"
            "}"
            "#program-status QPushButton[screenAction=\"true\"] {"
            "  background: %21;"
            "  border: 1px solid %22;"
            "  border-radius: 4px;"
            "  color: %3;"
            "  font-size: 10px;"
            "  font-weight: 600;"
            "  padding: 4px 6px;"
            "}"
            "#program-status QPushButton[screenAction=\"true\"]:hover {"
            "  background: %23;"
            "}"
            "#memory-status-label {"
            "  color: %17;"
            "  font-size: 10px;"
            "  background: transparent;"
            "}"
            "#cues-filter-bar {"
            "  background-color: %34;"
            "  border-top: 1px solid %16;"
            "}"
            "#quick-cues-shortcut-hint {"
            "  color: %35;"
            "  font-size: 10px;"
            "  border: none;"
            "  background: transparent;"
            "}"
            "#program-status QListWidget {"
            "  background-color: %32;"
            "  border: none;"
            "  color: %17;"
            "  font-size: 11px;"
            "  outline: 0;"
            "}"
            "#program-status QListWidget::item {"
            "  padding: 5px 8px;"
            "  border-bottom: 1px solid %36;"
            "}"
            "#program-status QListWidget::item:selected {"
            "  background-color: %37;"
            "  color: %38;"
            "}"
            "#program-status QListWidget::item:hover {"
            "  background-color: %39;"
            "}"
            "#program-status QLineEdit {"
            "  background-color: %20;"
            "  border: 1px solid %33;"
            "  border-radius: 4px;"
            "  color: %3;"
            "  font-size: 11px;"
            "  padding: 3px 8px;"
            "}"
            "#program-status QLineEdit:focus {"
            "  border-color: %14;"
            "}")
            .arg(p.status_panel_bg.name(),     // %1
                 p.border_strong.name(),       // %2
                 p.text_primary.name(),        // %3
                 p.header_bg.name(),           // %4
                 p.text_muted.name(),          // %5
                 p.monitor_row_bg.name(),      // %6
                 p.quick_clear_bg.name(),      // %7
                 p.quick_clear_border.name(),  // %8
                 p.quick_clear_hover_bg.name(),// %9
                 p.quick_clear_pressed_bg.name(),// %10
                 p.rail_bg.name(),             // %11
                 p.rail_border.name(),         // %12
                 p.rail_hover_bg.name(),       // %13
                 p.accent.name(),              // %14
                 p.toolbar_bg.name(),          // %15
                 p.border_mid.name(),          // %16
                 p.text_secondary.name(),      // %17
                 p.menu_hover_bg.name(),       // %18
                 p.toolbar_bg.name(),          // %19
                 p.input_alt_bg.name(),        // %20
                 p.button_bg.name(),           // %21
                 p.button_border.name(),       // %22
                 p.button_hover_bg.name(),     // %23
                 p.button_hover_border.name(), // %24
                 p.button_pressed_bg.name(),   // %25
                 p.tab_bar_bg.name(),          // %26
                 p.panel_alt_bg.name(),        // %27
                 p.sub_header_bg.name(),       // %28
                 p.text_link.name(),           // %29
                 p.text_link_hover.name(),     // %30
                 p.panel_bg.name(),            // %31
                 p.list_bg.name(),             // %32
                 p.border_subtle.name(),       // %33
                 p.filter_bar_bg.name(),       // %34
                 p.text_subtle.name(),         // %35
                 p.border_divider.name(),      // %36
                 p.list_selected_bg.name(),    // %37
                 p.text_selected.name(),       // %38
                 p.list_hover_bg.name()));     // %39

    quick_clear_button_->setIcon(make_glyph_icon(IconGlyph::clear_all, p.quick_clear_fg, 12));
    screen_menu_button_->setIcon(make_glyph_icon(IconGlyph::chevron_down, p.icon_muted, 12));
    previous_button_->setIcon(make_glyph_icon(IconGlyph::previous, p.icon_neutral, 11));
    next_button_->setIcon(make_glyph_icon(IconGlyph::next, p.icon_neutral, 11));
    clear_button_->setIcon(make_glyph_icon(IconGlyph::clear_layer, p.icon_neutral, 11));
    if (next_up_header_button_ != nullptr) {
        next_up_header_button_->setIcon(make_glyph_icon(IconGlyph::arrow_right, p.icon_muted, 13));
    }

    set_active_tab(active_tab_);
    set_live_details(live_details_);
    set_memory_status(last_working_set_bytes_, last_live_media_admitted_);
    if (slide_preview_ != nullptr) {
        slide_preview_->update();
    }
}

}  // namespace shekinah::ui
