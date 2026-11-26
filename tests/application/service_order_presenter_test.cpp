#include <catch2/catch_test_macros.hpp>

#include "shekinah/application/service_order_presenter.hpp"

using namespace shekinah::application;
using namespace shekinah::domain;

namespace {

Song make_song() {
    return Song{
        SongId{"song-a"},
        "Goodness",
        BackgroundRef{AssetId{"background"}},
        {
            Slide{CueId{"one"}, "Verse", {"One"}, std::nullopt},
            Slide{CueId{"two"}, "Chorus", {"Two"}, std::nullopt},
        },
    };
}

SermonDeck make_sermon() {
    return SermonDeck{
        SermonId{"sermon-a"},
        "Hope",
        {SermonSlide{CueId{"slide"}, AssetId{"image"}}},
    };
}

}  // namespace

TEST_CASE("service-order presenter describes a loaded Song") {
    const auto song = make_song();
    const auto item = ServiceOrderItem{
        ServiceOrderItemId{"item-1"}, ContentRef::song("song-a"), 0};

    const auto presented = present_service_order_item(item, &song, nullptr, ProgramState{});

    REQUIRE(presented.label == "Goodness");
    REQUIRE(presented.slide_count == 2);
    REQUIRE(presented.has_background);
    REQUIRE_FALSE(presented.is_live);
}

TEST_CASE("service-order presenter uses a stable fallback for a missing Song") {
    const auto item = ServiceOrderItem{
        ServiceOrderItemId{"item-1"}, ContentRef::song("song-a"), 0};

    const auto presented = present_service_order_item(item, nullptr, nullptr, ProgramState{});

    REQUIRE(presented.label == "Song song-a");
    REQUIRE(presented.slide_count == 0);
    REQUIRE_FALSE(presented.has_background);
}

TEST_CASE("service-order presenter describes Sermons and their live state") {
    const auto sermon = make_sermon();
    const auto item = ServiceOrderItem{
        ServiceOrderItemId{"item-1"}, ContentRef::sermon("sermon-a"), 0};
    const auto program = ProgramState{
        ProgramMode::cue,
        CueRef{ContentRef::sermon("sermon-a"), CueId{"slide"}},
    };

    const auto presented = present_service_order_item(item, nullptr, &sermon, program);

    REQUIRE(presented.label == "Hope");
    REQUIRE(presented.slide_count == 1);
    REQUIRE_FALSE(presented.has_background);
    REQUIRE(presented.is_live);
}

TEST_CASE("service-order presenter does not mark different content live") {
    const auto song = make_song();
    const auto item = ServiceOrderItem{
        ServiceOrderItemId{"item-1"}, ContentRef::song("song-a"), 0};
    const auto program = ProgramState{
        ProgramMode::cue,
        CueRef{ContentRef::song("song-other"), CueId{"one"}},
    };

    const auto presented = present_service_order_item(item, &song, nullptr, program);

    REQUIRE_FALSE(presented.is_live);
}
