#include "shekinah/ui/support/theme.hpp"

namespace shekinah::ui {

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

}  // namespace shekinah::ui
