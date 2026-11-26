#include <catch2/catch_test_macros.hpp>

#include "shekinah/application/live_slide_presenter.hpp"

using namespace shekinah::application;
using namespace shekinah::domain;

TEST_CASE("live Song presentation resolves slide text, navigation, and background") {
    const Song song{
        SongId{"song"},
        "Goodness",
        BackgroundRef{AssetId{"default-background"}, FitMode::cover},
        {
            Slide{CueId{"verse"}, "Verse", {"One"}, std::nullopt},
            Slide{CueId{"chorus"}, "Chorus", {"Two", "Three"},
                  BackgroundRef{AssetId{"chorus-background"}, FitMode::contain}},
            Slide{CueId{"bridge"}, "Bridge", {"Four"}, std::nullopt},
        },
    };

    const auto presented = build_live_song_presentation(song, CueId{"chorus"});

    REQUIRE(presented.item_title == "Goodness");
    REQUIRE(presented.slide_index == 2);
    REQUIRE(presented.total_slides == 3);
    REQUIRE(presented.section == "Chorus");
    REQUIRE(presented.slide_text == "Two\nThree");
    REQUIRE(presented.background_asset == AssetId{"chorus-background"});
    REQUIRE(presented.background_fit == FitMode::contain);
    REQUIRE(presented.next_section == "Bridge");
    REQUIRE(presented.next_summary == "Four");
}

TEST_CASE("live Song presentation leaves the media state empty for an unknown cue") {
    const Song song{
        SongId{"song"},
        "Goodness",
        std::nullopt,
        {Slide{CueId{"verse"}, "Verse", {"One"}, std::nullopt}},
    };

    const auto presented = build_live_song_presentation(song, CueId{"missing"});

    REQUIRE(presented.item_title == "Goodness");
    REQUIRE(presented.total_slides == 1);
    REQUIRE(presented.slide_index == 0);
    REQUIRE(presented.slide_text.empty());
    REQUIRE_FALSE(presented.background_asset.has_value());
}

TEST_CASE("live Sermon presentation exposes numbered navigation and image media") {
    const SermonDeck sermon{
        SermonId{"sermon"},
        "Hope",
        {
            SermonSlide{CueId{"one"}, AssetId{"image-one"}},
            SermonSlide{CueId{"two"}, AssetId{"image-two"}},
        },
    };

    const auto presented = build_live_sermon_presentation(sermon, CueId{"one"});

    REQUIRE(presented.item_title == "Hope");
    REQUIRE(presented.slide_index == 1);
    REQUIRE(presented.total_slides == 2);
    REQUIRE(presented.section == "Slide 1");
    REQUIRE(presented.background_asset == AssetId{"image-one"});
    REQUIRE(presented.next_section == "Slide 2");
    REQUIRE(presented.next_summary == "Next sermon slide");
}
