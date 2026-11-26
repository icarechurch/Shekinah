#include "shekinah/ui/support/theme.hpp"

#include <QSettings>

namespace shekinah::ui {
namespace {

constexpr auto kThemeSettingsKey = "ui/theme";

}  // namespace

void ThemeManager::load_from_settings(const QSettings& settings) {
    const QString key = settings.value(QString::fromLatin1(kThemeSettingsKey),
                                       QStringLiteral("dark"))
                            .toString();
    if (!set_theme_by_key(key)) {
        set_theme(ThemeId::dark);
    }
}

void ThemeManager::save_to_settings(QSettings& settings) const {
    settings.setValue(QString::fromLatin1(kThemeSettingsKey), info_for(current_id_).key);
}

}  // namespace shekinah::ui
