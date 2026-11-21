#include <catch2/catch_test_macros.hpp>

#include <QApplication>
#include <QAbstractItemView>
#include <QImage>
#include <QListView>
#include <QMimeData>
#include <QListWidget>

#include "test_project.hpp"
#include "shekinah/ui/library_dock_widget.hpp"
#include "shekinah/ui/support/asset_drag.hpp"
#include "shekinah/ui/support/content_drag.hpp"

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

TEST_CASE("reusable content drag payload round-trips a Song reference") {
    ensure_qapplication();
    const auto payload = shekinah::ui::make_content_mime_data(ContentRef::song("song-42"));

    const auto parsed = shekinah::ui::content_ref_from_mime_data(payload);

    REQUIRE(parsed.has_value());
    REQUIRE(parsed.value() == ContentRef::song("song-42"));
    delete payload;
}

TEST_CASE("Songs and Sermons library lists are draggable") {
    ensure_qapplication();
    LibraryDockWidget dock;
    dock.add_song(Song{SongId{"song-1"}, "Song", std::nullopt, {}});
    dock.add_sermon(SermonDeck{SermonId{"sermon-1"}, "Sermon", {}});

    auto* songs = dock.findChild<QListWidget*>(QStringLiteral("songs-list"));
    auto* sermons = dock.findChild<QListWidget*>(QStringLiteral("sermons-list"));
    REQUIRE(songs != nullptr);
    REQUIRE(sermons != nullptr);
    REQUIRE(songs->dragEnabled());
    REQUIRE(sermons->dragEnabled());
    REQUIRE(songs->dragDropMode() == QAbstractItemView::DragOnly);
    REQUIRE(sermons->dragDropMode() == QAbstractItemView::DragOnly);
}

TEST_CASE("library presents assets as a wrapping tile grid") {
    ensure_qapplication();
    LibraryDockWidget dock;
    dock.add_asset(AssetRecord{AssetId{"asset-1"}, AssetKind::image, "hash", "one.png", 100});

    auto* list = dock.findChild<QListWidget*>(QStringLiteral("assets-list"));
    REQUIRE(list != nullptr);
    REQUIRE(list->viewMode() == QListView::IconMode);
    REQUIRE(list->flow() == QListView::LeftToRight);
    REQUIRE(list->isWrapping());
    REQUIRE(list->movement() == QListView::Static);
    REQUIRE(list->dragEnabled());
    REQUIRE(list->dragDropMode() == QAbstractItemView::DragOnly);
}

TEST_CASE("library presents video assets with a media fallback icon") {
    ensure_qapplication();
    LibraryDockWidget dock;
    dock.add_asset(AssetRecord{AssetId{"asset-video"}, AssetKind::video, "hash", "loop.mp4", 200});

    auto* list = dock.findChild<QListWidget*>(QStringLiteral("assets-list"));
    REQUIRE(list != nullptr);
    REQUIRE(list->item(0)->text() == QStringLiteral("loop.mp4"));
    REQUIRE_FALSE(list->item(0)->icon().isNull());
}

TEST_CASE("asset tiles preserve ids and filter by filename") {
    ensure_qapplication();
    LibraryDockWidget dock;
    dock.add_asset(AssetRecord{AssetId{"asset-one"}, AssetKind::image, "h1", "media/one.png", 10});
    dock.add_asset(AssetRecord{AssetId{"asset-two"}, AssetKind::image, "h2", "media/two.png", 20});
    auto* list = dock.findChild<QListWidget*>(QStringLiteral("assets-list"));
    REQUIRE(list != nullptr);
    REQUIRE(list->item(0)->data(Qt::UserRole + 1).toString() == QStringLiteral("asset-one"));

    dock.set_search_text(QStringLiteral("two"));
    REQUIRE(list->item(0)->isHidden());
    REQUIRE_FALSE(list->item(1)->isHidden());
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
    const auto media_directory = project.root() / "media";
    std::filesystem::create_directories(media_directory);
    const auto image_path = media_directory / "slide.png";
    QImage image{320, 180, QImage::Format_ARGB32};
    image.fill(Qt::blue);
    REQUIRE(image.save(QString::fromStdString(image_path.string()), "PNG"));

    LibraryDockWidget dock;
    dock.set_asset_root(project.root());
    dock.add_asset(
        AssetRecord{AssetId{"asset-image"}, AssetKind::image, "hash", "media/slide.png", 128});

    auto* list = dock.findChild<QListWidget*>(QStringLiteral("assets-list"));
    REQUIRE(list != nullptr);
    REQUIRE(list->count() == 1);
    REQUIRE(list->item(0)->text() == QStringLiteral("slide.png"));
    REQUIRE(list->item(0)->toolTip().contains(QStringLiteral("media/slide.png")));
    REQUIRE(list->item(0)->toolTip().contains(QStringLiteral("128 bytes")));
    REQUIRE_FALSE(list->item(0)->icon().isNull());
    const auto thumbnail_size = list->item(0)->icon().availableSizes().front();
    REQUIRE(thumbnail_size.width() <= 168);
    REQUIRE(thumbnail_size.height() <= 94);
    REQUIRE(thumbnail_size.width() >= 160);
    REQUIRE(thumbnail_size.height() >= 90);
}
