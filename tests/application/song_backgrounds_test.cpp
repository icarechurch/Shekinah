#include <catch2/catch_test_macros.hpp>

#include "shekinah/application/song_backgrounds.hpp"

using namespace shekinah::application;
using namespace shekinah::domain;

TEST_CASE("slide background assignment applies from the selected slide forward") {
    Song song{SongId{"song"},
              "Song",
              std::nullopt,
              {Slide{CueId{"one"}, "Verse", {"One"}, std::nullopt},
               Slide{CueId{"two"}, "Chorus", {"Two"}, BackgroundRef{AssetId{"old"}}},
               Slide{CueId{"three"}, "Bridge", {"Three"}, std::nullopt}}};

    REQUIRE(apply_background_from_slide(
        song, CueId{"two"}, BackgroundRef{AssetId{"new"}, FitMode::cover}));
    REQUIRE_FALSE(song.slides[0].background.has_value());
    REQUIRE(song.slides[1].background->asset == AssetId{"new"});
    REQUIRE(song.slides[2].background->asset == AssetId{"new"});
}

TEST_CASE("slide background assignment rejects an unknown cue without mutation") {
    Song song{SongId{"song"},
              "Song",
              std::nullopt,
              {Slide{CueId{"one"}, "Verse", {"One"}, std::nullopt}}};

    REQUIRE_FALSE(apply_background_from_slide(
        song, CueId{"missing"}, BackgroundRef{AssetId{"new"}}));
    REQUIRE_FALSE(song.slides[0].background.has_value());
}
