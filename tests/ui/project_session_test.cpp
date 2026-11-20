#include <catch2/catch_test_macros.hpp>

#include <QApplication>

#include <fstream>

#include "test_project.hpp"
#include "shekinah/ui/main_window.hpp"

namespace {

QApplication& ensure_qapplication() {
    static int argc = 1;
    static char application_name[] = "shekinah_project_session_tests";
    static char* argv[] = {application_name, nullptr};
    static QApplication application{argc, argv};
    return application;
}

}  // namespace

TEST_CASE("main window opens a migrated SQLite project session") {
    ensure_qapplication();
    auto project = TestProject::create();
    shekinah::ui::ShekinahWindow window;

    REQUIRE(window.open_project(project.root() / "Sunday"));
    REQUIRE(window.project_open());
    REQUIRE(window.project_root() == project.root() / "Sunday");
}

TEST_CASE("main window imports assets through the project session") {
    ensure_qapplication();
    auto project = TestProject::create();
    const auto source = project.root() / "background.png";
    std::ofstream{source, std::ios::binary} << "png fixture";
    shekinah::ui::ShekinahWindow window;
    REQUIRE(window.open_project(project.root() / "Sunday"));

    const auto imported = window.import_asset_file(source);

    REQUIRE(imported);
    REQUIRE(std::filesystem::path{imported->relative_path}.parent_path() == "assets");
}
