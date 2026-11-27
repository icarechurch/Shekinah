#include "shekinah/ui/main_window.hpp"

namespace shekinah::ui {

void ShekinahWindow::apply_theme() {
    const auto& p = ThemeManager::instance().palette();
    setPalette(ThemeManager::build_qpalette(p));
    setStyleSheet(
        QStringLiteral(
            "QMainWindow, QWidget#operator-page {"
            "  background-color: %1;"
            "  color: %2;"
            "}"
            "QMenuBar {"
            "  background-color: %3;"
            "  color: %4;"
            "  border-bottom: 1px solid %5;"
            "}"
            "QMenuBar::item {"
            "  background: transparent;"
            "  padding: 4px 10px;"
            "}"
            "QMenuBar::item:selected {"
            "  background-color: %6;"
            "}"
            "QMenu {"
            "  background-color: %7;"
            "  color: %2;"
            "  border: 1px solid %8;"
            "}"
            "QMenu::item {"
            "  padding: 5px 22px 5px 12px;"
            "}"
            "QMenu::item:selected {"
            "  background-color: %9;"
            "  color: %10;"
            "}"
            "QMenu::separator {"
            "  height: 1px;"
            "  background: %8;"
            "  margin: 4px 8px;"
            "}"
            "QSplitter::handle {"
            "  background-color: %5;"
            "}"
            "QSplitter::handle:hover {"
            "  background-color: %9;"
            "}"
            "QDockWidget {"
            "  color: %2;"
            "  titlebar-close-icon: none;"
            "}"
            "QDockWidget::title {"
            "  background-color: %11;"
            "  padding: 5px 8px;"
            "  border-top: 1px solid %5;"
            "  border-bottom: 1px solid %5;"
            "  font-weight: 600;"
            "}"
            "QScrollBar:vertical {"
            "  background: %12;"
            "  width: 8px;"
            "  margin: 0;"
            "}"
            "QScrollBar::handle:vertical {"
            "  background: %13;"
            "  border-radius: 4px;"
            "  min-height: 24px;"
            "}"
            "QScrollBar::handle:vertical:hover {"
            "  background: %14;"
            "}"
            "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
            "  height: 0px;"
            "}"
            "QScrollBar:horizontal {"
            "  background: %12;"
            "  height: 8px;"
            "  margin: 0;"
            "}"
            "QScrollBar::handle:horizontal {"
            "  background: %13;"
            "  border-radius: 4px;"
            "  min-width: 24px;"
            "}"
            "QScrollBar::handle:horizontal:hover {"
            "  background: %14;"
            "}"
            "QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {"
            "  width: 0px;"
            "}"
            "QToolTip {"
            "  background-color: %7;"
            "  color: %2;"
            "  border: 1px solid %8;"
            "  padding: 3px 6px;"
            "}"
            "QMessageBox, QInputDialog {"
            "  background-color: %1;"
            "  color: %2;"
            "}"
            "QMessageBox QLabel, QInputDialog QLabel {"
            "  color: %2;"
            "  background: transparent;"
            "}"
            "QMessageBox QPushButton, QInputDialog QPushButton {"
            "  background-color: %15;"
            "  border: 1px solid %16;"
            "  border-radius: 4px;"
            "  color: %2;"
            "  padding: 5px 12px;"
            "  font-weight: 600;"
            "}"
            "QMessageBox QPushButton:hover, QInputDialog QPushButton:hover {"
            "  background-color: %17;"
            "  border-color: %18;"
            "}")
            .arg(p.window_bg.name(),               // %1
                 p.text_primary.name(),            // %2
                 p.menubar_bg.name(),              // %3
                 p.text_secondary.name(),          // %4
                 p.border_strong.name(),           // %5
                 p.menu_hover_bg.name(),           // %6
                 p.menu_bg.name(),                 // %7
                 p.border_subtle.name(),           // %8
                 p.accent.name(),                  // %9
                 p.text_on_accent.name(),          // %10
                 p.header_bg.name(),               // %11
                 p.scrollbar_track.name(),         // %12
                 p.scrollbar_handle.name(),        // %13
                 p.scrollbar_handle_hover.name(),  // %14
                 p.button_bg.name(),               // %15
                 p.button_border.name(),           // %16
                 p.button_hover_bg.name(),         // %17
                 p.button_hover_border.name()));   // %18
}

}  // namespace shekinah::ui
