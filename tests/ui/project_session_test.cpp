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

TEST_CASE("main window imports reusable song text through the project session") {
    ensure_qapplication();
    auto project = TestProject::create();
    const auto source = project.root() / "goodness.txt";
    std::ofstream{source} << "# Goodness\n[Verse 1]\nYou are good\n\n[Chorus]\nAll the time\n";
    shekinah::ui::ShekinahWindow window;
    REQUIRE(window.open_project(project.root() / "Sunday"));

    const auto imported = window.import_song_file(source);

    REQUIRE(imported);
    REQUIRE(imported->title == "Goodness");
    REQUIRE(imported->slides.size() == 2);
    REQUIRE(!imported->id.empty());
}

TEST_CASE("main window imports a reusable PNG sermon deck through the project session") {
    ensure_qapplication();
    auto project = TestProject::create();
    const auto slide_two = project.root() / "Slide 2.png";
    const auto slide_one = project.root() / "Slide 1.png";
    std::ofstream{slide_two, std::ios::binary} << "slide two";
    std::ofstream{slide_one, std::ios::binary} << "slide one";
    shekinah::ui::ShekinahWindow window;
    REQUIRE(window.open_project(project.root() / "Sunday"));

    const auto imported = window.import_sermon_files({slide_two, slide_one}, "Sunday Message");

    REQUIRE(imported);
    REQUIRE(imported->title == "Sunday Message");
    REQUIRE(imported->slides.size() == 2);
    REQUIRE(!imported->id.empty());
}
