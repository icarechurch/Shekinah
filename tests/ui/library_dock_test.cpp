#include <catch2/catch_test_macros.hpp>

#include <QApplication>

#include "shekinah/ui/library_dock_widget.hpp"

using namespace shekinah::domain;
using namespace shekinah::ui;

namespace {

QApplication& ensure_qapplication() {
    static int argc = 1;
    static char application_name[] = "shekinah_library_tests";
    static char* argv[] = {application_name, nullptr};
    static QApplication application{argc, argv};
    return application;
}

}  // namespace

TEST_CASE("library tabs retain assets when switching to reusable content") {
    ensure_qapplication();
    LibraryDockWidget dock;
    dock.add_asset(AssetRecord{AssetId{"asset-1"}, AssetKind::image, "hash-1", "assets/one.png", 100});
    dock.add_asset(AssetRecord{AssetId{"asset-2"}, AssetKind::video, "hash-2", "assets/two.mp4", 200});
    dock.show_assets();
    REQUIRE(dock.visible_asset_count() == 2);
    dock.show_songs();
    dock.show_sermons();
    dock.show_assets();
    REQUIRE(dock.visible_asset_count() == 2);
}

TEST_CASE("library owns independent reusable Song and Sermon collections") {
    ensure_qapplication();
    LibraryDockWidget dock;
    dock.add_song(Song{SongId{"song-a"}, "Goodness", std::nullopt, {}});
    dock.add_sermon(SermonDeck{SermonId{"sermon-a"}, "Sunday", {}});

    dock.show_songs();
    REQUIRE(dock.visible_content_count() == 1);
    dock.show_sermons();
    REQUIRE(dock.visible_content_count() == 1);
}
