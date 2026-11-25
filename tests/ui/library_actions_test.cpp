#include <catch2/catch_test_macros.hpp>

#include <QApplication>
#include <QListWidget>
#include <QPushButton>
#include <QTest>

#include "shekinah/ui/library_dock_widget.hpp"

namespace {

QApplication& ensure_qapplication() {
    static int argc = 1;
    static char application_name[] = "shekinah_library_action_tests";
    static char* argv[] = {application_name, nullptr};
    static QApplication application{argc, argv};
    return application;
}

}  // namespace

TEST_CASE("library exposes explicit authoring and import actions") {
    ensure_qapplication();
    shekinah::ui::LibraryDockWidget dock;
    int new_song = 0;
    int import_song = 0;
    int new_sermon = 0;
    int import_sermon = 0;
    int import_asset = 0;
    dock.set_new_song_handler([&] { ++new_song; });
    dock.set_import_song_handler([&] { ++import_song; });
    dock.set_new_sermon_handler([&] { ++new_sermon; });
    dock.set_import_sermon_handler([&] { ++import_sermon; });
    dock.set_import_asset_handler([&] { ++import_asset; });

    dock.findChild<QPushButton*>(QStringLiteral("new-song"))->click();
    dock.findChild<QPushButton*>(QStringLiteral("import-song-text"))->click();
    dock.findChild<QPushButton*>(QStringLiteral("new-sermon"))->click();
    dock.findChild<QPushButton*>(QStringLiteral("import-sermon-deck"))->click();
    dock.findChild<QPushButton*>(QStringLiteral("import-asset"))->click();

    REQUIRE(new_song == 1);
    REQUIRE(import_song == 1);
    REQUIRE(new_sermon == 1);
    REQUIRE(import_sermon == 1);
    REQUIRE(import_asset == 1);
}

TEST_CASE("library exposes an edit boundary for reusable Songs") {
    ensure_qapplication();
    shekinah::ui::LibraryDockWidget dock;
    dock.add_song(shekinah::domain::Song{
        shekinah::domain::SongId{"song-a"}, "Goodness", std::nullopt, {}});
    shekinah::domain::SongId edited;
    dock.set_edit_song_handler([&](shekinah::domain::SongId id) { edited = std::move(id); });

    dock.edit_song_row(0);

    REQUIRE(edited == shekinah::domain::SongId{"song-a"});
}

TEST_CASE("library exposes a visible Edit Song action") {
    ensure_qapplication();
    shekinah::ui::LibraryDockWidget dock;
    dock.add_song(shekinah::domain::Song{
        shekinah::domain::SongId{"song-a"}, "Goodness", std::nullopt, {}});
    shekinah::domain::SongId edited;
    dock.set_edit_song_handler([&](shekinah::domain::SongId id) { edited = std::move(id); });
    dock.show_songs();
    dock.findChild<QListWidget*>(QStringLiteral("songs-list"))->setCurrentRow(0);

    auto* edit = dock.findChild<QPushButton*>(QStringLiteral("edit-song"));
    REQUIRE(edit != nullptr);
    edit->click();

    REQUIRE(edited == shekinah::domain::SongId{"song-a"});
}

TEST_CASE("library exposes an Edit Sermon boundary") {
    ensure_qapplication();
    shekinah::ui::LibraryDockWidget dock;
    dock.add_sermon(shekinah::domain::SermonDeck{
        shekinah::domain::SermonId{"sermon-a"}, "Sunday", {}});
    shekinah::domain::SermonId edited;
    dock.set_edit_sermon_handler([&](shekinah::domain::SermonId id) { edited = std::move(id); });
    dock.show_sermons();
    dock.findChild<QListWidget*>(QStringLiteral("sermons-list"))->setCurrentRow(0);

    auto* edit = dock.findChild<QPushButton*>(QStringLiteral("edit-sermon"));
    REQUIRE(edit != nullptr);
    edit->click();

    REQUIRE(edited == shekinah::domain::SermonId{"sermon-a"});
}

TEST_CASE("library edit buttons disable without selection and quick-add card action adds content") {
    ensure_qapplication();
    shekinah::ui::LibraryDockWidget dock;
    dock.resize(1200, 220);
    dock.show();

    auto* edit_song = dock.findChild<QPushButton*>(QStringLiteral("edit-song"));
    auto* edit_sermon = dock.findChild<QPushButton*>(QStringLiteral("edit-sermon"));
    REQUIRE(edit_song != nullptr);
    REQUIRE(edit_sermon != nullptr);
    REQUIRE_FALSE(edit_song->isEnabled());
    REQUIRE_FALSE(edit_sermon->isEnabled());

    dock.add_song(shekinah::domain::Song{
        shekinah::domain::SongId{"song-a"}, "Goodness", std::nullopt, {}});
    dock.show_songs();
    QTest::qWait(20);

    auto* songs = dock.findChild<QListWidget*>(QStringLiteral("songs-list"));
    REQUIRE(songs != nullptr);
    songs->setCurrentRow(0);
    REQUIRE(edit_song->isEnabled());

    shekinah::domain::ContentRef added = shekinah::domain::ContentRef::sermon("unset");
    dock.set_add_content_handler([&](shekinah::domain::ContentRef content) { added = std::move(content); });
    const QRect card = songs->visualItemRect(songs->item(0)).adjusted(3, 3, -3, -3);
    const QPoint add_pill_center{card.right() - 28, card.center().y()};
    QTest::mouseClick(songs->viewport(), Qt::LeftButton, Qt::NoModifier, add_pill_center);

    REQUIRE(added == shekinah::domain::ContentRef::song("song-a"));
}
