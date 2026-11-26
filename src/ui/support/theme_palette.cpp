#include "shekinah/ui/support/theme.hpp"

#include "theme_palettes.hpp"

namespace shekinah::ui {

const ThemePalette& ThemeManager::palette_for(ThemeId id) {
    static const ThemePalette kDark = theme_internal::make_dark_palette();
    static const ThemePalette kLight = theme_internal::make_light_palette();
    static const ThemePalette kMidnight = theme_internal::make_midnight_palette();
    static const ThemePalette kSlate = theme_internal::make_slate_palette();
    static const ThemePalette kWarmLight = theme_internal::make_warm_light_palette();
    static const ThemePalette kHighContrast = theme_internal::make_high_contrast_palette();

    switch (resolve_theme_id(id)) {
    case ThemeId::light:
        return kLight;
    case ThemeId::midnight:
        return kMidnight;
    case ThemeId::slate:
        return kSlate;
    case ThemeId::warm_light:
        return kWarmLight;
    case ThemeId::high_contrast:
        return kHighContrast;
    case ThemeId::system:
    case ThemeId::dark:
        return kDark;
    }
    return kDark;
}

}  // namespace shekinah::ui
