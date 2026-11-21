#include <catch2/catch_test_macros.hpp>

#include <QApplication>
#include <QImage>
#include <QMimeData>
#include <QListWidget>

#include "test_project.hpp"
#include "shekinah/ui/library_dock_widget.hpp"
#include "shekinah/ui/support/asset_drag.hpp"

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

TEST_CASE("asset drag payload round-trips an AssetId") {
    ensure_qapplication();
    const auto payload = shekinah::ui::make_asset_mime_data(AssetId{"asset-42"});

    REQUIRE(payload != nullptr);
    REQUIRE(payload->hasFormat(shekinah::ui::asset_mime_type()));
    const auto parsed = shekinah::ui::asset_id_from_mime_data(payload);
    REQUIRE(parsed.has_value());
    REQUIRE(parsed->value == "asset-42");
    delete payload;
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

TEST_CASE("library clears all catalogs for a project reload") {
    ensure_qapplication();
    LibraryDockWidget dock;
    dock.add_asset(AssetRecord{AssetId{"asset-1"}, AssetKind::image, "hash-1", "assets/one.png", 100});
    dock.add_song(Song{SongId{"song-a"}, "Goodness", std::nullopt, {}});
    dock.add_sermon(SermonDeck{SermonId{"sermon-a"}, "Sunday", {}});

    dock.clear_contents();

    dock.show_assets();
    REQUIRE(dock.visible_asset_count() == 0);
    dock.show_songs();
    REQUIRE(dock.visible_content_count() == 0);
    dock.show_sermons();
    REQUIRE(dock.visible_content_count() == 0);
}

TEST_CASE("library creates bounded thumbnails for image assets") {
    ensure_qapplication();
    const auto project = TestProject::create();
    const auto image_path = project.root() / "slide.png";
    QImage image{320, 180, QImage::Format_ARGB32};
    image.fill(Qt::blue);
    REQUIRE(image.save(QString::fromStdString(image_path.string()), "PNG"));

    LibraryDockWidget dock;
    dock.set_asset_root(project.root());
    dock.add_asset(AssetRecord{AssetId{"asset-image"}, AssetKind::image, "hash", "slide.png", 128});

    auto* list = dock.findChild<QListWidget*>(QStringLiteral("assets-list"));
    REQUIRE(list != nullptr);
    REQUIRE(list->count() == 1);
    REQUIRE_FALSE(list->item(0)->icon().isNull());
}
