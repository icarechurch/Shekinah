#include "shekinah/ui/main_window.hpp"

#include "main_window_stylesheet.hpp"

namespace shekinah::ui {

void ShekinahWindow::apply_theme() {
    const auto& p = ThemeManager::instance().palette();
    setPalette(ThemeManager::build_qpalette(p));
    setStyleSheet(main_window_internal::make_main_window_stylesheet(p));
}

}  // namespace shekinah::ui
