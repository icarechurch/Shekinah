#include "shekinah/ui/operator_page.hpp"

#include "shekinah/ui/support/theme.hpp"

namespace shekinah::ui {

void OperatorPage::apply_theme() {
    const auto& p = ThemeManager::instance().palette();
    setStyleSheet(
        QStringLiteral(
            "#operator-page, #presentation-pane {"
            "  background-color: %1;"
            "  color: %2;"
            "}"
            "#presentation-section {"
            "  background-color: %3;"
            "  border-bottom: 1px solid %4;"
            "}"
            "#presentation-section-title {"
            "  color: %5;"
            "  font-size: 10px;"
            "  font-weight: 700;"
            "  letter-spacing: 1.1px;"
            "  border: none;"
            "  background: transparent;"
            "}"
            "#presentation-active-item {"
            "  color: %6;"
            "  font-size: 11px;"
            "  font-weight: 600;"
            "  border: none;"
            "  background: transparent;"
            "}")
            .arg(p.window_bg.name(),
                 p.text_primary.name(),
                 p.header_bg.name(),
                 p.border_strong.name(),
                 p.text_muted.name(),
                 p.text_secondary.name()));
}

}  // namespace shekinah::ui
