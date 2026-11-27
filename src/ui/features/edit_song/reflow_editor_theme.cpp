#include "shekinah/ui/reflow_editor_dialog.hpp"

#include "shekinah/ui/support/theme.hpp"

namespace shekinah::ui {

void ReflowEditorDialog::apply_theme() {
    const auto& p = ThemeManager::instance().palette();
    setPalette(ThemeManager::build_qpalette(p));
    setStyleSheet(ThemeManager::dialog_stylesheet(p));
}

}  // namespace shekinah::ui
