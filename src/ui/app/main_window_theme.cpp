#include "shekinah/ui/main_window.hpp"

#include <QActionGroup>

namespace shekinah::ui {

void ShekinahWindow::set_theme(ThemeId id) {
    ThemeManager::instance().set_theme(id);
}

ThemeId ShekinahWindow::theme() const noexcept {
    return ThemeManager::instance().current_theme_id();
}

void ShekinahWindow::sync_theme_actions() {
    if (theme_actions_ == nullptr) {
        return;
    }
    const int current = static_cast<int>(ThemeManager::instance().current_theme_id());
    for (auto* action : theme_actions_->actions()) {
        if (action != nullptr && action->data().toInt() == current) {
            action->setChecked(true);
        }
    }
}

}  // namespace shekinah::ui
