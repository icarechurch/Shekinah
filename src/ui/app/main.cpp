#include <QApplication>
#include <QStandardPaths>

#include <filesystem>

#include "shekinah/ui/main_window.hpp"

int main(int argc, char* argv[]) {
    QApplication application(argc, argv);
    shekinah::ui::ShekinahWindow window;
    const auto project_root = std::filesystem::path{
        QStandardPaths::writableLocation(QStandardPaths::AppDataLocation).toStdString()} /
        "projects" / "default";
    window.open_project(project_root);
    window.show();
    return application.exec();
}
