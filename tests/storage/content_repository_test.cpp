#include <catch2/catch_test_macros.hpp>

#include "test_project.hpp"
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
