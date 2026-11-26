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

TEST_CASE("main window imports a reusable mixed-media sermon deck through the project session") {
    ensure_qapplication();
    auto project = TestProject::create();
    const auto slide_two = project.root() / "Slide 2.GIF";
    const auto slide_one = project.root() / "Slide 1.MP4";
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

TEST_CASE("New Sermon opens an ordered slide authoring dialog and saves to the Library") {
    ensure_qapplication();
    auto project = TestProject::create();
    const auto slide = project.root() / "sermon-slide.png";
    QImage image{320, 180, QImage::Format_RGB32};
    image.fill(Qt::white);
    REQUIRE(image.save(QString::fromStdString(slide.string()), "PNG"));

    shekinah::ui::ShekinahWindow window;
    REQUIRE(window.open_project(project.root() / "Sunday"));
    window.show();
    auto* new_sermon = window.findChild<QPushButton*>(QStringLiteral("new-sermon"));
    REQUIRE(new_sermon != nullptr);
    QTest::mouseClick(new_sermon, Qt::LeftButton);

    auto* editor_widget = window.findChild<QWidget*>(QStringLiteral("sermon-editor-dialog"));
    REQUIRE(editor_widget != nullptr);
    auto* editor = static_cast<shekinah::ui::SermonEditorDialog*>(editor_widget);
    editor->set_title(QStringLiteral("Sunday Message"));
    editor->set_files({slide});
    auto* buttons = editor->findChild<QDialogButtonBox*>(QStringLiteral("sermon-buttons"));
    REQUIRE(buttons != nullptr);
    QTest::mouseClick(buttons->button(QDialogButtonBox::Save), Qt::LeftButton);
    QTest::qWait(20);

    window.library_dock()->show_sermons();
    REQUIRE(window.library_dock()->visible_content_count() == 1);
}

TEST_CASE("editing a reusable Sermon persists its title and reordered slide list") {
    ensure_qapplication();
    auto project = TestProject::create();
    const auto first = project.root() / "first.png";
    const auto second = project.root() / "second.png";
    QImage image{320, 180, QImage::Format_RGB32};
    image.fill(Qt::white);
    REQUIRE(image.save(QString::fromStdString(first.string()), "PNG"));
    image.fill(Qt::yellow);
    REQUIRE(image.save(QString::fromStdString(second.string()), "PNG"));

    shekinah::ui::ShekinahWindow window;
    REQUIRE(window.open_project(project.root() / "Sunday"));
    const auto sermon = window.import_sermon_files({first, second}, "Sunday Message", true);
    REQUIRE(sermon);
    window.library_dock()->add_sermon(*sermon);
    window.library_dock()->show_sermons();
    window.library_dock()->findChild<QListWidget*>(QStringLiteral("sermons-list"))->setCurrentRow(0);
    window.library_dock()->findChild<QPushButton*>(QStringLiteral("edit-sermon"))->click();

    auto* editor_widget = window.findChild<QWidget*>(QStringLiteral("sermon-editor-dialog"));
    REQUIRE(editor_widget != nullptr);
    auto* editor = static_cast<shekinah::ui::SermonEditorDialog*>(editor_widget);
    editor->set_title(QStringLiteral("Sunday Message Revised"));
    editor->findChild<QListWidget*>(QStringLiteral("sermon-files"))->setCurrentRow(1);
    editor->findChild<QPushButton*>(QStringLiteral("sermon-move-up"))->click();
    auto* buttons = editor->findChild<QDialogButtonBox*>(QStringLiteral("sermon-buttons"));
    REQUIRE(buttons != nullptr);
    QTest::mouseClick(buttons->button(QDialogButtonBox::Save), Qt::LeftButton);
    QTest::qWait(20);

    window.library_dock()->show_sermons();
    auto* sermons = window.library_dock()->findChild<QListWidget*>(QStringLiteral("sermons-list"));
    REQUIRE(sermons != nullptr);
    REQUIRE(sermons->count() == 1);
    REQUIRE(sermons->item(0)->text() == QStringLiteral("Sunday Message Revised"));
}

TEST_CASE("Inspector edits a live Song cue through the operator context menu") {
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

    auto* menu = window.operator_page()->create_context_menu(song->slides[0].id);
    REQUIRE(menu != nullptr);
    REQUIRE(menu->actions().size() >= 2);
    menu->actions().at(1)->trigger();
    menu->deleteLater();
    QTest::qWait(20);

    auto* inspector = window.operator_page()->inspector();
    REQUIRE(inspector->isVisible());
    inspector->findChild<QLineEdit*>(QStringLiteral("inspector-section"))->setText(QStringLiteral("Bridge"));
    inspector->findChild<QPlainTextEdit*>(QStringLiteral("inspector-lyrics"))->setPlainText(QStringLiteral("Changed live"));
    auto* buttons = inspector->findChild<QDialogButtonBox*>(QStringLiteral("inspector-buttons"));
    REQUIRE(buttons != nullptr);
    QTest::mouseClick(buttons->button(QDialogButtonBox::Save), Qt::LeftButton);
    QTest::qWait(20);

    auto* cue = window.operator_page()->cue_grid()->findChild<QWidget*>(
        QStringLiteral("cue-%1").arg(QString::fromStdString(song->slides[0].id.value)));
    REQUIRE(cue != nullptr);
    REQUIRE(cue->property("cue-summary").toString().contains(QStringLiteral("Changed live")));
}
