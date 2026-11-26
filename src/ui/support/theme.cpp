#include "shekinah/ui/support/theme.hpp"

#include "theme_palettes.hpp"

#include <QApplication>
#include <QGuiApplication>
#include <QSettings>
#include <QStyleHints>

namespace shekinah::ui {
namespace {

constexpr auto kThemeSettingsKey = "ui/theme";

}  // namespace

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

QPalette ThemeManager::build_qpalette(const ThemePalette& p) {
    QPalette pal;
    pal.setColor(QPalette::Window, p.window_bg);
    pal.setColor(QPalette::WindowText, p.text_primary);
    pal.setColor(QPalette::Base, p.input_bg);
    pal.setColor(QPalette::AlternateBase, p.panel_alt_bg);
    pal.setColor(QPalette::ToolTipBase, p.menu_bg);
    pal.setColor(QPalette::ToolTipText, p.text_primary);
    pal.setColor(QPalette::Text, p.text_primary);
    pal.setColor(QPalette::PlaceholderText, p.text_subtle);
    pal.setColor(QPalette::Button, p.button_bg);
    pal.setColor(QPalette::ButtonText, p.text_primary);
    pal.setColor(QPalette::BrightText, p.text_selected);
    pal.setColor(QPalette::Highlight, p.accent);
    pal.setColor(QPalette::HighlightedText, p.text_on_accent);
    pal.setColor(QPalette::Link, p.text_link);
    pal.setColor(QPalette::Mid, p.border_subtle);
    pal.setColor(QPalette::Dark, p.border_strong);
    pal.setColor(QPalette::Shadow, p.border_strong);

    pal.setColor(QPalette::Disabled, QPalette::WindowText, p.text_disabled);
    pal.setColor(QPalette::Disabled, QPalette::Text, p.text_disabled);
    pal.setColor(QPalette::Disabled, QPalette::ButtonText, p.text_disabled);
    pal.setColor(QPalette::Disabled, QPalette::Button, p.button_disabled_bg);
    return pal;
}

QString ThemeManager::dialog_stylesheet(const ThemePalette& p) {
    return QStringLiteral(
               "QDialog {"
               "  background-color: %1;"
               "  color: %2;"
               "}"
               "QLabel {"
               "  color: %2;"
               "  background: transparent;"
               "}"
               "QLineEdit, QPlainTextEdit, QTextEdit {"
               "  background-color: %3;"
               "  color: %2;"
               "  border: 1px solid %4;"
               "  border-radius: 4px;"
               "  padding: 4px 8px;"
               "  selection-background-color: %5;"
               "  selection-color: %6;"
               "}"
               "QLineEdit:focus, QPlainTextEdit:focus, QTextEdit:focus, QComboBox:focus {"
               "  border-color: %5;"
               "}"
               "QComboBox {"
               "  background-color: %7;"
               "  color: %2;"
               "  border: 1px solid %4;"
               "  border-radius: 4px;"
               "  padding: 4px 8px;"
               "}"
               "QComboBox QAbstractItemView {"
               "  background-color: %8;"
               "  color: %2;"
               "  border: 1px solid %4;"
               "  selection-background-color: %5;"
               "  selection-color: %6;"
               "}"
               "QListWidget {"
               "  background-color: %9;"
               "  color: %2;"
               "  border: 1px solid %4;"
               "  border-radius: 4px;"
               "  outline: 0;"
               "}"
               "QListWidget::item {"
               "  padding: 5px 8px;"
               "}"
               "QListWidget::item:selected {"
               "  background-color: %10;"
               "  color: %11;"
               "}"
               "QListWidget::item:hover {"
               "  background-color: %12;"
               "}"
               "QPushButton {"
               "  background-color: %7;"
               "  border: 1px solid %13;"
               "  border-radius: 4px;"
               "  color: %2;"
               "  padding: 5px 12px;"
               "  font-weight: 600;"
               "}"
               "QPushButton:hover {"
               "  background-color: %14;"
               "  border-color: %15;"
               "}"
               "QPushButton:pressed {"
               "  background-color: %16;"
               "}"
               "QPushButton:disabled {"
               "  background-color: %17;"
               "  border-color: %18;"
               "  color: %19;"
               "}"
               "QSplitter::handle {"
               "  background-color: %20;"
               "}"
               "QSplitter::handle:hover {"
               "  background-color: %5;"
               "}")
        .arg(p.panel_bg.name(),               // %1
             p.text_primary.name(),           // %2
             p.input_bg.name(),               // %3
             p.border_subtle.name(),          // %4
             p.accent.name(),                 // %5
             p.text_on_accent.name(),         // %6
             p.button_bg.name(),              // %7
             p.menu_bg.name(),                // %8
             p.list_bg.name(),                // %9
             p.list_selected_bg.name(),       // %10
             p.text_selected.name(),          // %11
             p.list_hover_bg.name(),          // %12
             p.button_border.name(),          // %13
             p.button_hover_bg.name(),        // %14
             p.button_hover_border.name(),    // %15
             p.button_pressed_bg.name(),      // %16
             p.button_disabled_bg.name(),     // %17
             p.button_disabled_border.name(), // %18
             p.text_disabled.name(),          // %19
             p.border_strong.name());         // %20
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
