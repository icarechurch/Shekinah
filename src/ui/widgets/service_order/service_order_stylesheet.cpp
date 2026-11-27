#include "service_order_stylesheet.hpp"

namespace shekinah::ui::service_order_internal {

QString make_service_order_stylesheet(const ThemePalette& p) {
    return QStringLiteral(
               "#service-order {"
               "  background-color: %1;"
               "  border-right: 1px solid %2;"
               "  color: %3;"
               "}"
               "#service-order-header {"
               "  background-color: %4;"
               "  border-bottom: 1px solid %2;"
               "}"
               "#service-order-title {"
               "  color: %5;"
               "  font-size: 10px;"
               "  font-weight: 700;"
               "  letter-spacing: 1.1px;"
               "  border: none;"
               "  background: transparent;"
               "}"
               "#service-order-count {"
               "  background-color: %6;"
               "  color: %7;"
               "  border: 1px solid %8;"
               "  border-radius: 3px;"
               "  padding: 0px 6px;"
               "  font-size: 9px;"
               "  font-weight: 700;"
               "}"
               "#service-order-add {"
               "  background: transparent;"
               "  border: none;"
               "  border-radius: 3px;"
               "}"
               "#service-order-add:hover {"
               "  background-color: %9;"
               "}"
               "#service-order-add:disabled {"
               "  opacity: 0.4;"
               "}"
               "#service-order QListWidget#service-order-list {"
               "  background-color: %1;"
               "  border: none;"
               "  outline: 0;"
               "}"
               "#service-order-empty-state { background: transparent; }"
               "#service-order-empty-title {"
               "  color: %10;"
               "  font-size: 12px;"
               "  font-weight: 600;"
               "  background: transparent;"
               "}"
               "#service-order-empty-hint {"
               "  color: %11;"
               "  font-size: 11px;"
               "  background: transparent;"
               "}"
               "#service-order-controls {"
               "  background-color: %12;"
               "  border-top: 1px solid %13;"
               "}"
               "#service-order-filter-bar {"
               "  background-color: %14;"
               "  border-top: 1px solid %13;"
               "}"
               "#service-order QPushButton[orderTool=\"true\"] {"
               "  background-color: %15;"
               "  border: 1px solid %16;"
               "  border-radius: 4px;"
               "  color: %17;"
               "  font-size: 11px;"
               "  font-weight: 600;"
               "  padding: 2px 6px;"
               "}"
               "#service-order QPushButton[orderTool=\"true\"]:hover {"
               "  background-color: %18;"
               "  border-color: %19;"
               "}"
               "#service-order QPushButton[orderTool=\"true\"]:pressed {"
               "  background-color: %20;"
               "}"
               "#service-order QPushButton[orderTool=\"true\"]:disabled {"
               "  background-color: %21;"
               "  border-color: %22;"
               "  color: %23;"
               "}"
               "#service-order QPushButton#service-order-remove:hover:enabled {"
               "  background-color: %24;"
               "  border-color: %25;"
               "  color: %26;"
               "}"
               "#service-order QLineEdit#service-order-filter {"
               "  background-color: %27;"
               "  border: 1px solid %28;"
               "  border-radius: 4px;"
               "  color: %3;"
               "  font-size: 11px;"
               "  padding: 3px 8px;"
               "}"
               "#service-order QLineEdit#service-order-filter:focus {"
               "  border-color: %29;"
               "}")
        .arg(p.panel_bg.name(),               // %1
             p.border_strong.name(),          // %2
             p.text_primary.name(),           // %3
             p.header_bg.name(),              // %4
             p.text_muted.name(),             // %5
             p.badge_bg.name(),               // %6
             p.badge_fg.name(),               // %7
             p.badge_border.name(),           // %8
             p.menu_hover_bg.name(),          // %9
             p.text_secondary.name(),         // %10
             p.text_subtle.name(),            // %11
             p.toolbar_bg.name(),             // %12
             p.border_mid.name(),             // %13
             p.filter_bar_bg.name(),          // %14
             p.button_alt_bg.name(),          // %15
             p.button_alt_border.name(),      // %16
             p.text_secondary.name(),         // %17
             p.button_hover_bg.name(),        // %18
             p.button_hover_border.name(),    // %19
             p.button_pressed_bg.name(),      // %20
             p.button_disabled_bg.name(),     // %21
             p.button_disabled_border.name(), // %22
             p.text_disabled.name(),          // %23
             p.danger_hover_bg.name(),        // %24
             p.danger_hover_border.name(),    // %25
             p.danger_text_on_bg.name(),      // %26
             p.input_bg.name(),               // %27
             p.border_subtle.name(),          // %28
             p.accent.name());                // %29
}

}  // namespace shekinah::ui::service_order_internal
