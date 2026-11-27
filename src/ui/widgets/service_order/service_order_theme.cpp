#include "shekinah/ui/service_order_widget.hpp"

#include "service_order_stylesheet.hpp"

#include "shekinah/ui/support/theme.hpp"

namespace shekinah::ui {
void ServiceOrderWidget::apply_theme() {
    const auto& p = ThemeManager::instance().palette();
    setStyleSheet(service_order_internal::make_service_order_stylesheet(p));
    apply_theme_icons();
}

}  // namespace shekinah::ui
