#include "shekinah/ui/support/theme.hpp"

#include <QApplication>
#include <QGuiApplication>
#include <QStyleHints>

namespace shekinah::ui {
ThemeManager::ThemeManager() {
    apply_resolved_theme(false);
    if (QGuiApplication::instance() != nullptr && QGuiApplication::styleHints() != nullptr) {
        QObject::connect(QGuiApplication::styleHints(),
                         &QStyleHints::colorSchemeChanged,
                         this,
                         [this](Qt::ColorScheme) {
                             if (current_id_ == ThemeId::system) {
                                 apply_resolved_theme(true);
                             }
                         });
    }
}

ThemeManager& ThemeManager::instance() {
    static ThemeManager manager;
    return manager;
}

void ThemeManager::set_theme(ThemeId id) {
    const ThemeId next_resolved = resolve_theme_id(id);
    if (current_id_ == id && resolved_id_ == next_resolved) {
        return;
    }
    current_id_ = id;
    apply_resolved_theme(true);
}

void ThemeManager::apply_resolved_theme(bool emit_signal) {
    resolved_id_ = resolve_theme_id(current_id_);
    current_palette_ = palette_for(resolved_id_);
    if (auto* app = qobject_cast<QApplication*>(QCoreApplication::instance()); app != nullptr) {
        app->setPalette(build_qpalette(current_palette_));
    }
    if (emit_signal) {
        emit theme_changed(current_id_);
    }
}

}  // namespace shekinah::ui
