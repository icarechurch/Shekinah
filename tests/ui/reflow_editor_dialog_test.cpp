#include <catch2/catch_test_macros.hpp>

#include <QApplication>
#include <QPushButton>
#include <QTest>

#include "shekinah/ui/reflow_editor_dialog.hpp"
#include "shekinah/ui/reflow_text_edit.hpp"

using namespace shekinah::ui;

namespace {

QApplication& ensure_qapplication() {
    static int argc = 1;
    static char application_name[] = "shekinah_reflow_tests";
    static char* argv[] = {application_name, nullptr};
    static QApplication application{argc, argv};
    return application;
}

}  // namespace

TEST_CASE("Ctrl Enter inserts a visible Reflow break without changing lyric text") {
    ensure_qapplication();
    ReflowEditorDialog dialog;
    dialog.set_text("First line\nSecond line");
    dialog.editor()->set_cursor_utf8_offset(10);
    QTest::keyClick(dialog.editor(), Qt::Key_Return, Qt::ControlModifier);

    REQUIRE(dialog.editor()->plain_text_utf8() == "First line\nSecond line");
    REQUIRE(dialog.editor()->visible_break_count() == 1);
    REQUIRE(dialog.reflow_document().to_slides().size() == 2);
    REQUIRE(dialog.thumbnail_count() == 2);
}

TEST_CASE("Backspace at the following text start merges the Reflow slides") {
    ensure_qapplication();
    ReflowEditorDialog dialog;
    dialog.set_text("First line\nSecond line");
    dialog.editor()->set_cursor_utf8_offset(10);
    QTest::keyClick(dialog.editor(), Qt::Key_Return, Qt::ControlModifier);
    dialog.editor()->set_cursor_utf8_offset(11);
    QTest::keyClick(dialog.editor(), Qt::Key_Backspace);

    REQUIRE(dialog.editor()->plain_text_utf8() == "First line\nSecond line");
    REQUIRE(dialog.editor()->visible_break_count() == 0);
    REQUIRE(dialog.reflow_document().to_slides().size() == 1);
}

TEST_CASE("Split Slide / Reflow button inserts a cursor slide break") {
    ensure_qapplication();
    ReflowEditorDialog dialog;
    dialog.set_text("First line\nSecond line");
    dialog.editor()->set_cursor_utf8_offset(10);

    auto* split_button = dialog.findChild<QPushButton*>(QStringLiteral("split-slide-reflow"));
    REQUIRE(split_button != nullptr);
    QTest::mouseClick(split_button, Qt::LeftButton);

    REQUIRE(dialog.editor()->plain_text_utf8() == "First line\nSecond line");
    REQUIRE(dialog.editor()->visible_break_count() == 1);
    REQUIRE(dialog.reflow_document().to_slides().size() == 2);
    REQUIRE(dialog.thumbnail_count() == 2);
}

TEST_CASE("Reflow editor validates and materializes a reusable Song draft") {
    ensure_qapplication();
    ReflowEditorDialog dialog;
    dialog.load_song(shekinah::domain::SongId{"song-a"});
    dialog.set_title(QStringLiteral("Goodness"));
    dialog.set_text("You are good\nAll the time");

    const auto draft = dialog.draft_song();

    REQUIRE(draft);
    REQUIRE(draft->id == shekinah::domain::SongId{"song-a"});
    REQUIRE(draft->title == "Goodness");
    REQUIRE(draft->slides.size() == 1);
    REQUIRE_FALSE(ReflowEditorDialog{}.draft_song());
}

TEST_CASE("Reflow draft uses lyric text typed after the document was loaded") {
    ensure_qapplication();
    ReflowEditorDialog dialog;
    dialog.load_song(shekinah::domain::SongId{"song-a"});
    dialog.set_title(QStringLiteral("Goodness"));
    dialog.set_text("Original lyric");
    dialog.editor()->setFocus();
    dialog.editor()->selectAll();
    QTest::keyClicks(dialog.editor(), QStringLiteral("Updated lyric"));

    const auto draft = dialog.draft_song();

    REQUIRE(draft);
    REQUIRE(draft->slides.size() == 1);
    REQUIRE(draft->slides[0].lines.size() == 1);
    REQUIRE(draft->slides[0].lines[0] == "Updated lyric");
}
