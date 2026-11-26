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

    auto* cue = window.operator_page()->cue_grid()->findChild<QWidget*>(
        QStringLiteral("cue-%1").arg(QString::fromStdString(song->slides[0].id.value)));
    REQUIRE(cue != nullptr);
    QTest::mouseClick(cue, Qt::LeftButton, Qt::NoModifier, cue->rect().center());

    REQUIRE(output->program().mode == shekinah::domain::ProgramMode::cue);
    REQUIRE(output->program().live_cue->cue == song->slides[0].id);
    REQUIRE(output->live_text() == QStringLiteral("You are good"));
    REQUIRE(window.operator_page()->program_status()->findChild<QWidget*>(
                QStringLiteral("program-preview"))
                ->property("slide-text")
                .toString() == QStringLiteral("You are good"));
}

TEST_CASE("Program Status previews a live song before Audience Output is created") {
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
    window.library_dock()->activate_song_row(0);
    window.show();
    QTest::qWait(20);

    auto* cue = window.operator_page()->cue_grid()->findChild<QWidget*>(
        QStringLiteral("cue-%1").arg(QString::fromStdString(song->slides[0].id.value)));
    REQUIRE(cue != nullptr);
    QTest::mouseClick(cue, Qt::LeftButton, Qt::NoModifier, cue->rect().center());

    REQUIRE(window.operator_page()->program_status()->findChild<QWidget*>(
                QStringLiteral("program-preview"))
                ->property("slide-text")
                .toString() == QStringLiteral("You are good"));
}

TEST_CASE("opening Audience Window after taking a song slide live displays the live slide") {
    ensure_qapplication();
    auto project = TestProject::create();
    const auto image_source = project.root() / "bg.png";
    QImage image{800, 450, QImage::Format_RGB32};
    image.fill(Qt::darkCyan);
    REQUIRE(image.save(QString::fromStdString(image_source.string()), "PNG"));

    const auto song_source = project.root() / "goodness.txt";
    std::ofstream{song_source} << "# Goodness\n[Verse]\nYou are good\n";

    shekinah::ui::ShekinahWindow window;
    REQUIRE(window.open_project(project.root() / "Sunday"));
    const auto asset = window.import_asset_file(image_source);
    const auto song = window.import_song_file(song_source);
    REQUIRE(asset);
    REQUIRE(song);
    window.library_dock()->add_song(*song);
    window.library_dock()->show_songs();
    window.library_dock()->activate_song_row(0);
    REQUIRE(window.set_slide_background(song->slides[0].id, asset->id));
    window.show();
    QTest::qWait(20);

    auto* cue = window.operator_page()->cue_grid()->findChild<QWidget*>(
        QStringLiteral("cue-%1").arg(QString::fromStdString(song->slides[0].id.value)));
    REQUIRE(cue != nullptr);
    QTest::mouseClick(cue, Qt::LeftButton, Qt::NoModifier, cue->rect().center());

    auto* window_btn = window.operator_page()->program_status()->findChild<QPushButton*>(
        QStringLiteral("output-window-btn"));
    REQUIRE(window_btn != nullptr);
    QTest::mouseClick(window_btn, Qt::LeftButton);
    QTest::qWait(20);

    auto* output = window.audience_output();
    REQUIRE(output->mode() == shekinah::media::OutputMode::movable);
    REQUIRE(output->program().mode == shekinah::domain::ProgramMode::cue);
    REQUIRE(output->program().live_cue.has_value());
    REQUIRE(output->program().live_cue->cue == song->slides[0].id);
    REQUIRE(output->live_text() == QStringLiteral("You are good"));
    REQUIRE(output->slide_image_loaded());
}

TEST_CASE("opening Audience Window after taking a sermon slide or Black live syncs output state") {
    ensure_qapplication();
    auto project = TestProject::create();
    const auto source = project.root() / "Slide 1.png";
    QImage image{800, 450, QImage::Format_RGB32};
    image.fill(Qt::blue);
    REQUIRE(image.save(QString::fromStdString(source.string()), "PNG"));

    SECTION("sermon slide is synced on first open") {
        shekinah::ui::ShekinahWindow window;
        REQUIRE(window.open_project(project.root() / "Sunday"));
        const auto sermon = window.import_sermon_files({source}, "Sunday Message");
        REQUIRE(sermon);
        window.library_dock()->add_sermon(*sermon);
        window.library_dock()->show_sermons();
        window.library_dock()->activate_sermon_row(0);
        window.show();
        QTest::qWait(20);

        auto* cue = window.operator_page()->cue_grid()->findChild<QWidget*>(QStringLiteral("cue-slide-0"));
        REQUIRE(cue != nullptr);
        QTest::mouseClick(cue, Qt::LeftButton, Qt::NoModifier, cue->rect().center());

        auto* window_btn = window.operator_page()->program_status()->findChild<QPushButton*>(
            QStringLiteral("output-window-btn"));
        REQUIRE(window_btn != nullptr);
        QTest::mouseClick(window_btn, Qt::LeftButton);
        QTest::qWait(20);

        auto* output = window.audience_output();
        REQUIRE(output->program().mode == shekinah::domain::ProgramMode::cue);
        REQUIRE(output->slide_image_loaded());
    }

    SECTION("black program mode is synced on first open") {
        shekinah::ui::ShekinahWindow window;
        REQUIRE(window.open_project(project.root() / "SundayBlack"));
        window.show();
        QTest::qWait(20);

        auto* black = window.operator_page()->program_status()->findChild<QPushButton*>(QStringLiteral("black"));
        REQUIRE(black != nullptr);
        QTest::mouseClick(black, Qt::LeftButton);

        auto* window_btn = window.operator_page()->program_status()->findChild<QPushButton*>(
            QStringLiteral("output-window-btn"));
        REQUIRE(window_btn != nullptr);
        QTest::mouseClick(window_btn, Qt::LeftButton);
        QTest::qWait(20);

        auto* output = window.audience_output();
        REQUIRE(output->program().mode == shekinah::domain::ProgramMode::black);
    }
}

TEST_CASE("a live sermon cue loads its imported PNG into Audience Output") {
    ensure_qapplication();
    auto project = TestProject::create();
    const auto source = project.root() / "Slide 1.png";
    QImage image{1600, 900, QImage::Format_RGB32};
    image.fill(Qt::blue);
    REQUIRE(image.save(QString::fromStdString(source.string()), "PNG"));

    shekinah::ui::ShekinahWindow window;
    REQUIRE(window.open_project(project.root() / "Sunday"));
    const auto sermon = window.import_sermon_files({source}, "Sunday Message");
    REQUIRE(sermon);
    window.library_dock()->add_sermon(*sermon);
    window.library_dock()->show_sermons();
    window.library_dock()->activate_sermon_row(0);
    auto* output = window.audience_output();
    window.show();
    QTest::qWait(20);

    auto* cue = window.operator_page()->cue_grid()->findChild<QWidget*>(QStringLiteral("cue-slide-0"));
    REQUIRE(cue != nullptr);
    QTest::mouseClick(cue, Qt::LeftButton, Qt::NoModifier, cue->rect().center());

    REQUIRE(output->slide_image_loaded());
}

TEST_CASE("New Song saves a Reflow draft into the reusable Songs library") {
    ensure_qapplication();
    auto project = TestProject::create();
    shekinah::ui::ShekinahWindow window;
    REQUIRE(window.open_project(project.root() / "Sunday"));
    window.show();

    auto* new_song = window.findChild<QPushButton*>(QStringLiteral("new-song"));
    REQUIRE(new_song != nullptr);
    QTest::mouseClick(new_song, Qt::LeftButton);
    auto* editor_widget = window.findChild<QWidget*>(QStringLiteral("reflow-editor-dialog"));
    REQUIRE(editor_widget != nullptr);
    auto* editor = static_cast<shekinah::ui::ReflowEditorDialog*>(editor_widget);
    editor->set_title(QStringLiteral("New Creation"));
    editor->set_text("First line\nSecond line");
    auto* buttons = editor->findChild<QDialogButtonBox*>(QStringLiteral("song-editor-buttons"));
    REQUIRE(buttons != nullptr);
    QTest::mouseClick(buttons->button(QDialogButtonBox::Save), Qt::LeftButton);
    QTest::qWait(20);

    window.library_dock()->show_songs();
    REQUIRE(window.library_dock()->visible_content_count() == 1);
}

TEST_CASE("New Song remains in SQLite after reopening the project") {
    ensure_qapplication();
    auto project = TestProject::create();
    shekinah::ui::ShekinahWindow window;
    REQUIRE(window.open_project(project.root() / "Sunday"));
    window.show();

    auto* new_song = window.findChild<QPushButton*>(QStringLiteral("new-song"));
    REQUIRE(new_song != nullptr);
    QTest::mouseClick(new_song, Qt::LeftButton);
    auto* editor_widget = window.findChild<QWidget*>(QStringLiteral("reflow-editor-dialog"));
    REQUIRE(editor_widget != nullptr);
    auto* editor = static_cast<shekinah::ui::ReflowEditorDialog*>(editor_widget);
    editor->set_title(QStringLiteral("Persisted Song"));
    editor->set_text("Saved line");
    auto* buttons = editor->findChild<QDialogButtonBox*>(QStringLiteral("song-editor-buttons"));
    REQUIRE(buttons != nullptr);
    QTest::mouseClick(buttons->button(QDialogButtonBox::Save), Qt::LeftButton);
    QTest::qWait(20);

    shekinah::ui::ShekinahWindow reopened;
    REQUIRE(reopened.open_project(project.root() / "Sunday"));
    reopened.library_dock()->show_songs();
    REQUIRE(reopened.library_dock()->visible_content_count() == 1);
}

TEST_CASE("editing a reusable Song loads and persists its stored Reflow document") {
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
    window.library_dock()->edit_song_row(0);
    auto* editor_widget = window.findChild<QWidget*>(QStringLiteral("reflow-editor-dialog"));
    REQUIRE(editor_widget != nullptr);
    auto* editor = static_cast<shekinah::ui::ReflowEditorDialog*>(editor_widget);
    REQUIRE(editor->editor()->visible_break_count() == 1);
    editor->set_title(QStringLiteral("Goodness Rewritten"));
    editor->set_text("New verse\nNew chorus");
    editor->editor()->set_cursor_utf8_offset(9);
    REQUIRE(editor->editor()->insert_slide_break());
    auto* buttons = editor->findChild<QDialogButtonBox*>(QStringLiteral("song-editor-buttons"));
    REQUIRE(buttons != nullptr);
    QTest::mouseClick(buttons->button(QDialogButtonBox::Save), Qt::LeftButton);
    QTest::qWait(20);

    auto* songs = window.library_dock()->findChild<QListWidget*>(QStringLiteral("songs-list"));
    REQUIRE(songs != nullptr);
    REQUIRE(songs->count() == 1);
    REQUIRE(songs->item(0)->text() == "Goodness Rewritten");
}

TEST_CASE("typed lyric edits are persisted and restored from SQLite") {
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
    window.library_dock()->edit_song_row(0);

    auto* editor_widget = window.findChild<QWidget*>(QStringLiteral("reflow-editor-dialog"));
    REQUIRE(editor_widget != nullptr);
    auto* editor = static_cast<shekinah::ui::ReflowEditorDialog*>(editor_widget);
    editor->editor()->setFocus();
    editor->editor()->selectAll();
    QTest::keyClicks(editor->editor(), QStringLiteral("Edited lyric from operator"));
    auto* buttons = editor->findChild<QDialogButtonBox*>(QStringLiteral("song-editor-buttons"));
    REQUIRE(buttons != nullptr);
    QTest::mouseClick(buttons->button(QDialogButtonBox::Save), Qt::LeftButton);
    QTest::qWait(20);

    shekinah::ui::ShekinahWindow reopened;
    REQUIRE(reopened.open_project(project.root() / "Sunday"));
    reopened.library_dock()->show_songs();
    reopened.library_dock()->activate_song_row(0);
    reopened.show();
    QTest::qWait(20);
    auto* cue = reopened.operator_page()->cue_grid()->findChild<QWidget*>(
        QStringLiteral("cue-%1").arg(QString::fromStdString(song->slides[0].id.value)));
    REQUIRE(cue != nullptr);
    REQUIRE(cue->property("cue-summary").toString().contains(
        QStringLiteral("Edited lyric from operator")));
}

TEST_CASE("global Program controls navigate, clear, and black the live output") {
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
    auto* output = window.audience_output();
    window.show();
    QTest::qWait(20);

    auto* first_cue = window.operator_page()->cue_grid()->findChild<QWidget*>(
        QStringLiteral("cue-%1").arg(QString::fromStdString(song->slides[0].id.value)));
    REQUIRE(first_cue != nullptr);
    QTest::mouseClick(first_cue, Qt::LeftButton, Qt::NoModifier, first_cue->rect().center());
    auto* next = window.operator_page()->program_status()->findChild<QPushButton*>(QStringLiteral("next"));
    auto* previous = window.operator_page()->program_status()->findChild<QPushButton*>(QStringLiteral("previous"));
    auto* clear = window.operator_page()->program_status()->findChild<QPushButton*>(QStringLiteral("clear"));
    auto* black = window.operator_page()->program_status()->findChild<QPushButton*>(QStringLiteral("black"));
    REQUIRE(next != nullptr);
    REQUIRE(previous != nullptr);
    REQUIRE(clear != nullptr);
    REQUIRE(black != nullptr);

    QTest::mouseClick(next, Qt::LeftButton);
    REQUIRE(output->program().live_cue->cue == song->slides[1].id);
    QTest::mouseClick(previous, Qt::LeftButton);
    REQUIRE(output->program().live_cue->cue == song->slides[0].id);
    QTest::mouseClick(black, Qt::LeftButton);
    REQUIRE(output->program().mode == shekinah::domain::ProgramMode::black);
    QTest::mouseClick(clear, Qt::LeftButton);
    REQUIRE(output->program().mode == shekinah::domain::ProgramMode::clear);
}

TEST_CASE("generated reusable content ids remain unique after project reopen") {
    ensure_qapplication();
    auto project = TestProject::create();
    const auto source = project.root() / "goodness.txt";
    std::ofstream{source} << "# Goodness\n[Verse]\nYou are good\n";

    shekinah::ui::ShekinahWindow first_window;
    REQUIRE(first_window.open_project(project.root() / "Sunday"));
    const auto first = first_window.import_song_file(source);
    REQUIRE(first);

    shekinah::ui::ShekinahWindow reopened_window;
    REQUIRE(reopened_window.open_project(project.root() / "Sunday"));
    const auto second = reopened_window.import_song_file(source);

    REQUIRE(second);
    REQUIRE(first->id != second->id);
    REQUIRE(first->title == second->title);
}

TEST_CASE("an imported image can become a live Song cue background") {
    ensure_qapplication();
    auto project = TestProject::create();
    const auto image_source = project.root() / "background.png";
    QImage image{800, 450, QImage::Format_RGB32};
    image.fill(Qt::green);
    REQUIRE(image.save(QString::fromStdString(image_source.string()), "PNG"));
    const auto song_source = project.root() / "goodness.txt";
    std::ofstream{song_source} << "# Goodness\n[Verse]\nYou are good\n";

    shekinah::ui::ShekinahWindow window;
    REQUIRE(window.open_project(project.root() / "Sunday"));
    const auto asset = window.import_asset_file(image_source);
    const auto song = window.import_song_file(song_source);
    REQUIRE(asset);
    REQUIRE(song);
    window.library_dock()->add_song(*song);
    window.library_dock()->show_songs();
    window.library_dock()->activate_song_row(0);
    REQUIRE(window.set_slide_background(song->slides[0].id, asset->id));
    auto* output = window.audience_output();
    window.show();
    QTest::qWait(20);

    auto* cue = window.operator_page()->cue_grid()->findChild<QWidget*>(
        QStringLiteral("cue-%1").arg(QString::fromStdString(song->slides[0].id.value)));
    REQUIRE(cue != nullptr);
    QTest::mouseClick(cue, Qt::LeftButton, Qt::NoModifier, cue->rect().center());

    REQUIRE(output->slide_image_loaded());
}

TEST_CASE("an imported MP4 can become a looping live Song cue background") {
    ensure_qapplication();
    auto project = TestProject::create();
    const auto video_source = project.root() / "motion.mp4";
    std::ofstream{video_source, std::ios::binary} << "mp4 fixture";
    const auto song_source = project.root() / "goodness.txt";
    std::ofstream{song_source} << "# Goodness\n[Verse]\nYou are good\n";

    shekinah::ui::ShekinahWindow window;
    REQUIRE(window.open_project(project.root() / "Sunday"));
    const auto asset = window.import_asset_file(video_source);
    const auto song = window.import_song_file(song_source);
    REQUIRE(asset);
    REQUIRE(song);
    window.library_dock()->add_song(*song);
    window.library_dock()->show_songs();
    window.library_dock()->activate_song_row(0);
    REQUIRE(window.set_slide_background(song->slides[0].id, asset->id));
    auto* output = window.audience_output();
    window.show();
    QTest::qWait(20);

    auto* cue = window.operator_page()->cue_grid()->findChild<QWidget*>(
        QStringLiteral("cue-%1").arg(QString::fromStdString(song->slides[0].id.value)));
    REQUIRE(cue != nullptr);
    QTest::mouseClick(cue, Qt::LeftButton, Qt::NoModifier, cue->rect().center());

    REQUIRE(output->loop_video_loaded());
    REQUIRE_FALSE(output->slide_image_loaded());
}
