#include <catch2/catch_test_macros.hpp>

#include <QApplication>
#include <QDialogButtonBox>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QImage>
#include <QLabel>
#include <QListWidget>
#include <QLineEdit>
#include <QMenu>
#include <QPushButton>
#include <QPlainTextEdit>
#include <QTest>

#include <algorithm>
#include <fstream>

#include "qt_drag_event_position.hpp"
#include "test_project.hpp"
#include "shekinah/media/audience_output.hpp"
#include "shekinah/ui/library_dock_widget.hpp"
#include "shekinah/ui/main_window.hpp"
#include "shekinah/ui/operator_page.hpp"
#include "shekinah/ui/program_status_widget.hpp"
#include "shekinah/ui/reflow_editor_dialog.hpp"
#include "shekinah/ui/reflow_text_edit.hpp"
#include "shekinah/ui/sermon_editor_dialog.hpp"
#include "shekinah/ui/service_order_widget.hpp"
#include "shekinah/ui/cue_grid_widget.hpp"
#include "shekinah/ui/support/asset_drag.hpp"
#include "shekinah/ui/support/content_drag.hpp"
#include "shekinah/storage/content_repository.hpp"
#include "shekinah/storage/project_store.hpp"

namespace {

QApplication& ensure_qapplication() {
    static int argc = 1;
    static char application_name[] = "shekinah_project_session_tests";
    static char* argv[] = {application_name, nullptr};
    static QApplication application{argc, argv};
    return application;
}

}  // namespace

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

TEST_CASE("dragging a Song into Presentations creates a removable reference") {
    ensure_qapplication();
    auto project = TestProject::create();
    const auto source = project.root() / "goodness.txt";
    std::ofstream{source} << "# Goodness\n[Verse]\nYou are good\n";

    shekinah::ui::ShekinahWindow window;
    REQUIRE(window.open_project(project.root() / "Sunday"));
    const auto song = window.import_song_file(source);
    REQUIRE(song);
    window.library_dock()->add_song(*song);
    window.library_dock()->show_songs();
    window.show();
    QTest::qWait(20);

    auto* songs = window.library_dock()->findChild<QListWidget*>(QStringLiteral("songs-list"));
    auto* presentations = window.operator_page()->service_order_widget()->findChild<QListWidget*>(
        QStringLiteral("service-order-list"));
    REQUIRE(songs != nullptr);
    REQUIRE(presentations != nullptr);
    REQUIRE(songs->count() == 1);
    REQUIRE(presentations->count() == 0);

    auto* mime = shekinah::ui::make_content_mime_data(
        shekinah::domain::ContentRef::song(song->id.value));
    const auto position =
        test_support::drag_event_position(presentations->viewport()->rect().center());
    QDragEnterEvent enter{position, Qt::CopyAction, mime, Qt::LeftButton, Qt::NoModifier};
    QApplication::sendEvent(presentations->viewport(), &enter);
    QDragMoveEvent move{position, Qt::CopyAction, mime, Qt::LeftButton, Qt::NoModifier};
    QApplication::sendEvent(presentations->viewport(), &move);
    QDropEvent drop{position, Qt::CopyAction, mime, Qt::LeftButton, Qt::NoModifier};
    QApplication::sendEvent(presentations->viewport(), &drop);
    delete mime;

    REQUIRE(drop.isAccepted());
    REQUIRE(presentations->count() == 1);
    REQUIRE(songs->count() == 1);

    window.operator_page()->service_order_widget()->select_row(0);
    auto* remove = window.operator_page()->service_order_widget()->findChild<QPushButton*>(
        QStringLiteral("service-order-remove"));
    REQUIRE(remove != nullptr);
    remove->click();
    QTest::qWait(20);

    REQUIRE(presentations->count() == 0);
    REQUIRE(songs->count() == 1);
    auto store = shekinah::storage::ProjectStore::create_or_open(project.root() / "Sunday");
    REQUIRE(store);
    shekinah::storage::SqliteContentRepository repository{*store};
    REQUIRE(repository.load_song(song->id));
}

TEST_CASE("service-order move controls persist the new reusable-content order") {
    ensure_qapplication();
    auto project = TestProject::create();
    const auto first_source = project.root() / "first.txt";
    const auto second_source = project.root() / "second.txt";
    std::ofstream{first_source} << "# First\n[Verse]\nFirst line\n";
    std::ofstream{second_source} << "# Second\n[Verse]\nSecond line\n";

    shekinah::ui::ShekinahWindow window;
    REQUIRE(window.open_project(project.root() / "Sunday"));
    const auto first = window.import_song_file(first_source);
    const auto second = window.import_song_file(second_source);
    REQUIRE(first);
    REQUIRE(second);
    window.library_dock()->add_song(*first);
    window.library_dock()->add_song(*second);
    window.library_dock()->show_songs();
    window.library_dock()->activate_song_row(0);
    window.library_dock()->activate_song_row(1);
    window.show();
    QTest::qWait(20);

    window.operator_page()->service_order_widget()->select_row(1);
    auto* move_up = window.operator_page()->service_order_widget()->findChild<QPushButton*>(
        QStringLiteral("service-order-move-up"));
    REQUIRE(move_up != nullptr);
    move_up->click();

    const auto& items = window.operator_page()->service_order_widget()->items();
    REQUIRE(items.size() == 2);
    REQUIRE(items[0].label == "Second");
    REQUIRE(items[1].label == "First");
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

    auto* chorus = window.operator_page()->cue_grid()->findChild<QWidget*>(
        QStringLiteral("cue-%1").arg(QString::fromStdString(song->slides[1].id.value)));
    REQUIRE(chorus != nullptr);
    QTest::mouseClick(chorus, Qt::LeftButton, Qt::NoModifier, chorus->rect().center());

    const auto* program_cue = window.operator_page()->program_status()->findChild<QLabel*>(
        QStringLiteral("program-cue"));
    REQUIRE(program_cue != nullptr);
    REQUIRE(program_cue->text() == QString::fromStdString(song->slides[1].id.value));
}
