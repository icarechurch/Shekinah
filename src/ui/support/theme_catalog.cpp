#include "shekinah/ui/support/theme.hpp"

namespace shekinah::ui {

const std::vector<ThemeInfo>& ThemeManager::available_themes() {
    static const std::vector<ThemeInfo> themes = {
        {ThemeId::system,
         QStringLiteral("system"),
         QStringLiteral("System (Auto)"),
         QStringLiteral("Follow the operating system light or dark appearance"),
         false},
        {ThemeId::dark,
         QStringLiteral("dark"),
         QStringLiteral("Dark"),
         QStringLiteral("Charcoal studio operator theme"),
         true},
        {ThemeId::light,
         QStringLiteral("light"),
         QStringLiteral("Light"),
         QStringLiteral("Crisp daylight workspace theme"),
         false},
        {ThemeId::midnight,
         QStringLiteral("midnight"),
         QStringLiteral("Midnight"),
         QStringLiteral("Ultra-low-glare theme for darkened booths"),
         true},
        {ThemeId::slate,
         QStringLiteral("slate"),
         QStringLiteral("Slate"),
         QStringLiteral("Cool Nordic blue-grey studio theme"),
         true},
        {ThemeId::warm_light,
         QStringLiteral("warm-light"),
         QStringLiteral("Warm Light"),
         QStringLiteral("Soft warm paper daylight theme"),
         false},
        {ThemeId::high_contrast,
         QStringLiteral("high-contrast"),
         QStringLiteral("High Contrast"),
         QStringLiteral("Accessible high-contrast dark theme"),
         true},
    };
    return themes;
}

ThemeInfo ThemeManager::info_for(ThemeId id) {
    for (const auto& info : available_themes()) {
        if (info.id == id) {
            return info;
        }
    }
    return available_themes().at(1);
}

}  // namespace shekinah::ui
