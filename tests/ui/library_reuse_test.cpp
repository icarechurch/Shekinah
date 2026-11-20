#include <catch2/catch_test_macros.hpp>

#include <QApplication>

#include "shekinah/ui/library_dock_widget.hpp"

using namespace shekinah::domain;
using namespace shekinah::ui;

namespace {

QApplication& ensure_qapplication() {
    static int argc = 1;
    static char application_name[] = "shekinah_library_reuse_tests";
    static char* argv[] = {application_name, nullptr};
    static QApplication application{argc, argv};
    return application;
}

}  // namespace

TEST_CASE("library double-click exposes Song and Sermon references for reuse") {
    ensure_qapplication();
    LibraryDockWidget dock;
    dock.add_song(Song{SongId{"song-a"}, "Goodness", std::nullopt, {}});
    dock.add_sermon(SermonDeck{SermonId{"sermon-a"}, "Sunday", {}});
    ContentRef added;
    dock.set_add_content_handler([&](ContentRef content) { added = std::move(content); });

    dock.activate_song_row(0);
    REQUIRE(added == ContentRef::song("song-a"));
    dock.activate_sermon_row(0);
    REQUIRE(added == ContentRef::sermon("sermon-a"));
}
