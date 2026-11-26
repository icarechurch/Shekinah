#include "shekinah/ui/support/theme.hpp"

#include <QGuiApplication>
#include <QStyleHints>

namespace shekinah::ui {

bool ThemeManager::set_theme_by_key(const QString& key) {
    const QString normalized = key.trimmed().toLower();
    for (const auto& info : available_themes()) {
        if (info.key == normalized) {
            set_theme(info.id);
            return true;
        }
    }
    return false;
}

ThemeId ThemeManager::resolve_theme_id(ThemeId id) {
    if (id != ThemeId::system) {
        return id;
    }
    if (QGuiApplication::instance() != nullptr && QGuiApplication::styleHints() != nullptr &&
        QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Light) {
        return ThemeId::light;
    }
    return ThemeId::dark;
}

}  // namespace shekinah::ui
