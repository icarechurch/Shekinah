#include "shekinah/ui/sermon_editor_dialog.hpp"

#include "shekinah/ui/support/theme.hpp"

namespace shekinah::ui {

void SermonEditorDialog::apply_theme() {
    const auto& p = ThemeManager::instance().palette();
    setPalette(ThemeManager::build_qpalette(p));
    setStyleSheet(ThemeManager::dialog_stylesheet(p));
}

}  // namespace shekinah::ui
