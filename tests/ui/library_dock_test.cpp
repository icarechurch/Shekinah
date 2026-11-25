#include <catch2/catch_test_macros.hpp>

#include <QApplication>
#include <QAbstractItemView>
#include <QImage>
#include <QLabel>
#include <QListView>
#include <QMimeData>
#include <QListWidget>
#include <QPushButton>
#include <QTest>

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

TEST_CASE("library segmented tabs, counts, and empty state track active content") {
    ensure_qapplication();
    LibraryDockWidget dock;
    dock.resize(1200, 220);
    dock.show();
    QTest::qWait(20);

    auto* total_badge = dock.findChild<QLabel*>(QStringLiteral("library-total-count"));
    auto* assets_tab = dock.findChild<QPushButton*>(QStringLiteral("library-tab-assets"));
    auto* songs_tab = dock.findChild<QPushButton*>(QStringLiteral("library-tab-songs"));
    auto* sermons_tab = dock.findChild<QPushButton*>(QStringLiteral("library-tab-sermons"));
    auto* empty_overlay = dock.findChild<QWidget*>(QStringLiteral("library-empty-state"));
    REQUIRE(total_badge != nullptr);
    REQUIRE(assets_tab != nullptr);
    REQUIRE(songs_tab != nullptr);
    REQUIRE(sermons_tab != nullptr);
    REQUIRE(empty_overlay != nullptr);

    REQUIRE(total_badge->text() == QStringLiteral("0 ITEMS"));
    REQUIRE(assets_tab->text() == QStringLiteral("Assets (0)"));
    REQUIRE(empty_overlay->isVisible());

    dock.add_asset(AssetRecord{AssetId{"a-1"}, AssetKind::video, "h1", "media/loop.mp4", 2048});
    dock.add_song(Song{SongId{"s-1"}, "Goodness", std::nullopt, {Slide{CueId{"c1"}, "Verse", {"Line"}, std::nullopt}}});
    dock.add_sermon(SermonDeck{SermonId{"d-1"}, "Sunday", {}});
    QTest::qWait(20);

    REQUIRE(total_badge->text() == QStringLiteral("3 ITEMS"));
    REQUIRE(assets_tab->text() == QStringLiteral("Assets (1)"));
    REQUIRE(songs_tab->text() == QStringLiteral("Songs (1)"));
    REQUIRE(sermons_tab->text() == QStringLiteral("Sermons (1)"));
    REQUIRE_FALSE(empty_overlay->isVisible());

    songs_tab->click();
    REQUIRE(dock.current_tab() == LibraryTab::songs);
    REQUIRE_FALSE(empty_overlay->isVisible());

    sermons_tab->click();
    REQUIRE(dock.current_tab() == LibraryTab::sermons);
    REQUIRE_FALSE(empty_overlay->isVisible());

    dock.clear_contents();
    REQUIRE(total_badge->text() == QStringLiteral("0 ITEMS"));
    REQUIRE(empty_overlay->isVisible());
}

TEST_CASE("library caches bounded thumbnails across refreshes and frees them on purge") {
    ensure_qapplication();
    const auto project = TestProject::create();
    const auto media_directory = project.root() / "media";
    std::filesystem::create_directories(media_directory);
    const auto image_path = media_directory / "bg.png";
    QImage image{1920, 1080, QImage::Format_ARGB32};
    image.fill(Qt::darkBlue);
    REQUIRE(image.save(QString::fromStdString(image_path.string()), "PNG"));

    LibraryDockWidget dock;
    dock.set_asset_root(project.root());
    const AssetRecord record{AssetId{"asset-bg"}, AssetKind::image, "hash-bg", "media/bg.png", 1024};
    dock.add_asset(record);
    REQUIRE(dock.cached_thumbnail_count() == 1);

    dock.clear_contents();
    REQUIRE(dock.cached_thumbnail_count() == 1);

    dock.add_asset(record);
    auto* list = dock.findChild<QListWidget*>(QStringLiteral("assets-list"));
    REQUIRE(list != nullptr);
    REQUIRE_FALSE(list->item(0)->icon().isNull());

    dock.purge_non_live_caches();
    REQUIRE(dock.cached_thumbnail_count() == 0);
}
