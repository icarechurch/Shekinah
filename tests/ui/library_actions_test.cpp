#include <catch2/catch_test_macros.hpp>

#include <QApplication>
#include <QPushButton>

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
