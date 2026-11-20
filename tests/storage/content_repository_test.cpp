#include <catch2/catch_test_macros.hpp>

#include <fstream>

#include "test_project.hpp"
#include "shekinah/storage/asset_importer.hpp"
#include "shekinah/storage/content_repository.hpp"
#include "shekinah/storage/project_store.hpp"

using namespace shekinah::domain;
using namespace shekinah::storage;

TEST_CASE("a SQLite content repository persists one reusable song and many order references") {
    auto project = TestProject::create();
    const auto project_root = project.root() / "Sunday";

    {
        auto store = ProjectStore::create_or_open(project_root);
        REQUIRE(store);
        SqliteContentRepository repository{*store};

        REQUIRE(repository.ensure_service_order(ServiceOrderId{"order-a"}, "Sunday"));
        const Song song{
            SongId{"song-a"},
            "Goodness of God",
            std::nullopt,
            {Slide{CueId{"verse-1"}, "Verse 1", {"I love You, Lord"}, std::nullopt}},
        };
        REQUIRE(repository.save_song(song));

        for (int index = 0; index != 500; ++index) {
            REQUIRE(repository.append_to_order(ServiceOrderId{"order-a"}, ContentRef::song("song-a")));
        }

        const auto order = repository.load_order(ServiceOrderId{"order-a"});
        REQUIRE(order);
        REQUIRE(order->size() == 500);
        REQUIRE(order->front().ordinal == 0);
        REQUIRE(order->back().ordinal == 499);
        const auto count = repository.song_count();
        REQUIRE(count);
        REQUIRE(*count == 1);
    }

    auto reopened = ProjectStore::create_or_open(project_root);
    REQUIRE(reopened);
    SqliteContentRepository repository{*reopened};
    const auto loaded_song = repository.load_song(SongId{"song-a"});
    REQUIRE(loaded_song);
    REQUIRE(loaded_song->title == "Goodness of God");
    REQUIRE(loaded_song->slides.size() == 1);
    REQUIRE(loaded_song->slides.front().lines.front() == "I love You, Lord");
    const auto order = repository.load_order(ServiceOrderId{"order-a"});
    REQUIRE(order);
    REQUIRE(order->size() == 500);
}

TEST_CASE("SQLite content repository refuses order references to missing content") {
    auto project = TestProject::create();
    auto store = ProjectStore::create_or_open(project.root() / "Sunday");
    REQUIRE(store);
    SqliteContentRepository repository{*store};
    REQUIRE(repository.ensure_service_order(ServiceOrderId{"order-a"}, "Sunday"));

    const auto result = repository.append_to_order(
        ServiceOrderId{"order-a"}, ContentRef::sermon("missing-sermon"));
    REQUIRE_FALSE(result);
    REQUIRE(result.error().code == ErrorCode::not_found);
}

TEST_CASE("SQLite content repository lists active reusable content and assets") {
    auto project = TestProject::create();
    auto store = ProjectStore::create_or_open(project.root() / "Sunday");
    REQUIRE(store);
    SqliteContentRepository repository{*store};
    REQUIRE(repository.save_song(Song{SongId{"song-z"}, "Z Song", std::nullopt, {}}));
    REQUIRE(repository.save_song(Song{SongId{"song-a"}, "A Song", std::nullopt, {}}));
    REQUIRE(repository.save_deck(SermonDeck{SermonId{"sermon-a"}, "A Sermon", {}}));

    const auto songs = repository.list_songs();
    REQUIRE(songs);
    REQUIRE(songs->size() == 2);
    REQUIRE(songs->front().title == "A Song");

    const auto sermons = repository.list_decks();
    REQUIRE(sermons);
    REQUIRE(sermons->size() == 1);
    REQUIRE(sermons->front().title == "A Sermon");

    const auto assets = repository.list_assets();
    REQUIRE(assets);
    REQUIRE(assets->empty());

    REQUIRE(repository.archive_song(SongId{"song-a"}));
    const auto active_songs = repository.list_songs();
    REQUIRE(active_songs);
    REQUIRE(active_songs->size() == 1);
    REQUIRE(active_songs->front().id == SongId{"song-z"});
}

TEST_CASE("SQLite content repository loads stored asset metadata by id") {
    auto project = TestProject::create();
    auto store = ProjectStore::create_or_open(project.root() / "Sunday");
    REQUIRE(store);
    const auto source = project.root() / "background.png";
    std::ofstream{source, std::ios::binary} << "png fixture";
    const auto imported = AssetImporter{*store}.import_file(source);
    REQUIRE(imported);
    SqliteContentRepository repository{*store};

    const auto loaded = repository.load_asset(imported->id);

    REQUIRE(loaded);
    REQUIRE(loaded->id == imported->id);
    REQUIRE(loaded->kind == AssetKind::image);
    REQUIRE(loaded->relative_path == imported->relative_path);
    REQUIRE(loaded->size_bytes == imported->size_bytes);
}

TEST_CASE("reusable content cannot be archived while a service order references it") {
    auto project = TestProject::create();
    auto store = ProjectStore::create_or_open(project.root() / "Sunday");
    REQUIRE(store);
    SqliteContentRepository repository{*store};
    REQUIRE(repository.ensure_service_order(ServiceOrderId{"order-a"}, "Sunday"));
    REQUIRE(repository.save_song(Song{SongId{"song-a"}, "Goodness", std::nullopt, {}}));
    const auto item = repository.append_to_order(ServiceOrderId{"order-a"}, ContentRef::song("song-a"));
    REQUIRE(item);

    REQUIRE_FALSE(repository.archive_song(SongId{"song-a"}));
    REQUIRE(repository.remove_from_order(item->id));
    REQUIRE(repository.archive_song(SongId{"song-a"}));
    REQUIRE_FALSE(repository.load_song(SongId{"song-a"}));
}

TEST_CASE("SQLite content repository moves a service item and rewrites contiguous ordinals") {
    auto project = TestProject::create();
    auto store = ProjectStore::create_or_open(project.root() / "Sunday");
    REQUIRE(store);
    SqliteContentRepository repository{*store};
    REQUIRE(repository.ensure_service_order(ServiceOrderId{"order-a"}, "Sunday"));
    REQUIRE(repository.save_song(Song{SongId{"song-a"}, "Song A", std::nullopt, {}}));
    REQUIRE(repository.save_song(Song{SongId{"song-b"}, "Song B", std::nullopt, {}}));
    REQUIRE(repository.save_song(Song{SongId{"song-c"}, "Song C", std::nullopt, {}}));
    const auto first = repository.append_to_order(ServiceOrderId{"order-a"}, ContentRef::song("song-a"));
    const auto second = repository.append_to_order(ServiceOrderId{"order-a"}, ContentRef::song("song-b"));
    const auto third = repository.append_to_order(ServiceOrderId{"order-a"}, ContentRef::song("song-c"));
    REQUIRE(first);
    REQUIRE(second);
    REQUIRE(third);

    REQUIRE(repository.move_in_order(third->id, 0));

    const auto order = repository.load_order(ServiceOrderId{"order-a"});
    REQUIRE(order);
    REQUIRE(order->size() == 3);
    REQUIRE(order->at(0).content == ContentRef::song("song-c"));
    REQUIRE(order->at(1).content == ContentRef::song("song-a"));
    REQUIRE(order->at(2).content == ContentRef::song("song-b"));
    REQUIRE(order->at(0).ordinal == 0);
    REQUIRE(order->at(1).ordinal == 1);
    REQUIRE(order->at(2).ordinal == 2);
}

TEST_CASE("SQLite content repository rejects an invalid service item move") {
    auto project = TestProject::create();
    auto store = ProjectStore::create_or_open(project.root() / "Sunday");
    REQUIRE(store);
    SqliteContentRepository repository{*store};
    REQUIRE(repository.ensure_service_order(ServiceOrderId{"order-a"}, "Sunday"));
    REQUIRE_FALSE(repository.move_in_order(ServiceOrderItemId{"missing"}, 0));
    REQUIRE_FALSE(repository.move_in_order(ServiceOrderItemId{"missing"}, -1));
}
