#include <catch2/catch_test_macros.hpp>

#include <QApplication>
#include <QLabel>
#include <QTest>

#include <fstream>

#include "test_project.hpp"
#include "shekinah/media/audience_output.hpp"
#include "shekinah/ui/library_dock_widget.hpp"
#include "shekinah/ui/main_window.hpp"
#include "shekinah/ui/operator_page.hpp"
#include "shekinah/ui/program_status_widget.hpp"
#include "shekinah/ui/service_order_widget.hpp"
#include "shekinah/ui/cue_grid_widget.hpp"

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

TEST_CASE("opening a project restores reusable content in the Library") {
    ensure_qapplication();
    auto project = TestProject::create();
    const auto source = project.root() / "goodness.txt";
    std::ofstream{source} << "# Goodness\n[Verse]\nYou are good\n";

    shekinah::ui::ShekinahWindow first_window;
    REQUIRE(first_window.open_project(project.root() / "Sunday"));
    REQUIRE(first_window.import_song_file(source));

    shekinah::ui::ShekinahWindow reopened_window;
    REQUIRE(reopened_window.open_project(project.root() / "Sunday"));
    reopened_window.library_dock()->show_songs();

    REQUIRE(reopened_window.library_dock()->visible_content_count() == 1);
}

TEST_CASE("repeated service-order references survive a project reopen") {
    ensure_qapplication();
    auto project = TestProject::create();
    const auto source = project.root() / "goodness.txt";
    std::ofstream{source} << "# Goodness\n[Verse]\nYou are good\n";

    shekinah::ui::ShekinahWindow first_window;
    REQUIRE(first_window.open_project(project.root() / "Sunday"));
    const auto song = first_window.import_song_file(source);
    REQUIRE(song);
    first_window.library_dock()->add_song(*song);
    first_window.library_dock()->show_songs();
    first_window.library_dock()->activate_song_row(0);
    first_window.library_dock()->activate_song_row(0);
    REQUIRE(first_window.operator_page()->service_order_widget()->count() == 2);

    shekinah::ui::ShekinahWindow reopened_window;
    REQUIRE(reopened_window.open_project(project.root() / "Sunday"));

    const auto& items = reopened_window.operator_page()->service_order_widget()->items();
    REQUIRE(items.size() == 2);
    REQUIRE(items[0].label == "Goodness");
    REQUIRE(items[1].label == "Goodness");
}

TEST_CASE("service-order selection rebuilds cues and clicking a cue changes Program") {
    ensure_qapplication();
    auto project = TestProject::create();
    const auto source = project.root() / "goodness.txt";
    std::ofstream{source} << "# Goodness\n[Verse]\nYou are good\n\n[Chorus]\nAll the time\n";

    shekinah::ui::ShekinahWindow window;
    REQUIRE(window.open_project(project.root() / "Sunday"));
    const auto song = window.import_song_file(source);
    REQUIRE(song);
    window.library_dock()->add_song(*song);
    window.library_dock()->show_songs();
    window.library_dock()->activate_song_row(0);
    window.show();
    QTest::qWait(20);

    auto* chorus = window.operator_page()->cue_grid()->findChild<QWidget*>(QStringLiteral("cue-slide-1"));
    REQUIRE(chorus != nullptr);
    QTest::mouseClick(chorus, Qt::LeftButton, Qt::NoModifier, chorus->rect().center());

    const auto* program_cue = window.operator_page()->program_status()->findChild<QLabel*>(
        QStringLiteral("program-cue"));
    REQUIRE(program_cue != nullptr);
    REQUIRE(program_cue->text() == QStringLiteral("slide-1"));
}

TEST_CASE("a live cue is mirrored to the Audience Output state") {
    ensure_qapplication();
    auto project = TestProject::create();
    const auto source = project.root() / "goodness.txt";
    std::ofstream{source} << "# Goodness\n[Verse]\nYou are good\n";

    shekinah::ui::ShekinahWindow window;
    REQUIRE(window.open_project(project.root() / "Sunday"));
    auto* output = window.audience_output();
    REQUIRE(output->mode() == shekinah::media::OutputMode::hidden);
    const auto song = window.import_song_file(source);
    REQUIRE(song);
    window.library_dock()->add_song(*song);
    window.library_dock()->show_songs();
    window.library_dock()->activate_song_row(0);
    window.show();
    QTest::qWait(20);

    auto* cue = window.operator_page()->cue_grid()->findChild<QWidget*>(QStringLiteral("cue-slide-0"));
    REQUIRE(cue != nullptr);
    QTest::mouseClick(cue, Qt::LeftButton, Qt::NoModifier, cue->rect().center());

    REQUIRE(output->program().mode == shekinah::domain::ProgramMode::cue);
    REQUIRE(output->program().live_cue->cue == shekinah::domain::CueId{"slide-0"});
}
