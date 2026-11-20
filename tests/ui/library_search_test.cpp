#include <catch2/catch_test_macros.hpp>

#include <QApplication>

#include "shekinah/ui/library_dock_widget.hpp"

using namespace shekinah::domain;
using namespace shekinah::ui;

namespace {

QApplication& ensure_qapplication() {
    static int argc = 1;
    static char application_name[] = "shekinah_library_search_tests";
    static char* argv[] = {application_name, nullptr};
    static QApplication application{argc, argv};
    return application;
}

}  // namespace

TEST_CASE("library search filters metadata without removing imported assets") {
    ensure_qapplication();
    LibraryDockWidget dock;
    dock.add_asset(AssetRecord{AssetId{"asset-one"}, AssetKind::image, "hash-one", "assets/sermon.png", 100});
    dock.add_asset(AssetRecord{AssetId{"asset-two"}, AssetKind::video, "hash-two", "assets/worship.mp4", 200});
    dock.show_assets();

    dock.set_search_text(QStringLiteral("sermon"));
    REQUIRE(dock.visible_asset_count() == 1);
    dock.set_search_text(QString{});
    REQUIRE(dock.visible_asset_count() == 2);
}
