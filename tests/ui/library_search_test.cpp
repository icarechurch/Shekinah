#include <catch2/catch_test_macros.hpp>

#include <QApplication>
#include <QPushButton>

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

TEST_CASE("library media-type pills filter images and videos independently of search") {
    ensure_qapplication();
    LibraryDockWidget dock;
    dock.add_asset(AssetRecord{AssetId{"asset-img"}, AssetKind::image, "h1", "assets/sky.png", 100});
    dock.add_asset(AssetRecord{AssetId{"asset-vid"}, AssetKind::video, "h2", "assets/sky_loop.mp4", 200});
    dock.show_assets();

    auto* filter_all = dock.findChild<QPushButton*>(QStringLiteral("filter-assets-all"));
    auto* filter_images = dock.findChild<QPushButton*>(QStringLiteral("filter-assets-images"));
    auto* filter_videos = dock.findChild<QPushButton*>(QStringLiteral("filter-assets-videos"));
    REQUIRE(filter_all != nullptr);
    REQUIRE(filter_images != nullptr);
    REQUIRE(filter_videos != nullptr);

    filter_images->click();
    REQUIRE(dock.asset_kind_filter() == AssetKind::image);
    REQUIRE(dock.visible_asset_count() == 1);

    filter_videos->click();
    REQUIRE(dock.asset_kind_filter() == AssetKind::video);
    REQUIRE(dock.visible_asset_count() == 1);

    dock.set_search_text(QStringLiteral("sky"));
    REQUIRE(dock.visible_asset_count() == 1);

    filter_all->click();
    REQUIRE_FALSE(dock.asset_kind_filter().has_value());
    REQUIRE(dock.visible_asset_count() == 2);
}
