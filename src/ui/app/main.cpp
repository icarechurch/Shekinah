#include <QApplication>
#include <QSettings>
#include <QStandardPaths>

#include <filesystem>

#include "shekinah/ui/main_window.hpp"
#include "shekinah/ui/support/app_icon.hpp"
#include "shekinah/ui/support/theme.hpp"

int main(int argc, char* argv[]) {
    QApplication application(argc, argv);
    application.setWindowIcon(shekinah::ui::app_icon());
    QSettings settings{QStringLiteral("Shekinah"), QStringLiteral("Shekinah")};
    shekinah::ui::ThemeManager::instance().load_from_settings(settings);
    QObject::connect(&shekinah::ui::ThemeManager::instance(),
                     &shekinah::ui::ThemeManager::theme_changed,
                     &application,
                     [&settings](shekinah::ui::ThemeId) {
                         shekinah::ui::ThemeManager::instance().save_to_settings(settings);
                     });

    shekinah::ui::ShekinahWindow window;
    const auto project_root = std::filesystem::path{
        QStandardPaths::writableLocation(QStandardPaths::AppDataLocation).toStdString()} /
        "projects" / "default";
    window.open_project(project_root);
    window.show();
    return application.exec();
}
