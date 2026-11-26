#include <catch2/catch_test_macros.hpp>

#include "shekinah/application/slide_insertion.hpp"

using namespace shekinah::application;
using namespace shekinah::domain;

TEST_CASE("inserting an asset slide into a Song supports before first, between, and after last") {
    Song song{SongId{"song-1"},
              "Goodness",
              std::nullopt,
              {Slide{CueId{"slide-0"}, "Verse", {"Line 1"}, std::nullopt},
               Slide{CueId{"slide-1"}, "Chorus", {"Line 2"}, BackgroundRef{AssetId{"bg-old"}}}}};

    const auto first = insert_asset_slide(song, 0, BackgroundRef{AssetId{"bg-intro"}, FitMode::cover});
    REQUIRE(first);
    REQUIRE(song.slides.size() == 3);
    REQUIRE(song.slides[0].id == *first);
    REQUIRE(song.slides[0].section == "Media");
    REQUIRE(song.slides[0].lines.empty());
    REQUIRE(song.slides[0].background->asset == AssetId{"bg-intro"});
    REQUIRE(song.slides[1].id == CueId{"slide-0"});
    REQUIRE_FALSE(song.slides[1].background.has_value());

    const auto middle = insert_asset_slide(song, 2, BackgroundRef{AssetId{"bg-mid"}, FitMode::contain});
    REQUIRE(middle);
    REQUIRE(song.slides.size() == 4);
    REQUIRE(song.slides[2].id == *middle);
    REQUIRE(song.slides[2].background->asset == AssetId{"bg-mid"});
    REQUIRE(song.slides[2].background->fit == FitMode::contain);
    REQUIRE(song.slides[3].id == CueId{"slide-1"});
    REQUIRE(song.slides[3].background->asset == AssetId{"bg-old"});

    const auto end = insert_asset_slide(song, song.slides.size(), BackgroundRef{AssetId{"bg-end"}});
    REQUIRE(end);
    REQUIRE(song.slides.size() == 5);
    REQUIRE(song.slides.back().id == *end);
    REQUIRE(song.slides.back().background->asset == AssetId{"bg-end"});
}

TEST_CASE("inserting an asset slide into a SermonDeck supports before first, between, and after last") {
    SermonDeck deck{SermonId{"sermon-1"},
                    "Sunday Message",
                    {SermonSlide{CueId{"slide-0"}, AssetId{"a-1"}},
                     SermonSlide{CueId{"slide-1"}, AssetId{"a-2"}}}};

    const auto before = insert_asset_slide(deck, 0, AssetId{"a-0"});
    REQUIRE(before);
    REQUIRE(deck.slides.size() == 3);
    REQUIRE(deck.slides[0].id == *before);
    REQUIRE(deck.slides[0].image == AssetId{"a-0"});
    REQUIRE(deck.slides[1].id == CueId{"slide-0"});

    const auto between = insert_asset_slide(deck, 2, AssetId{"a-mid"});
    REQUIRE(between);
    REQUIRE(deck.slides.size() == 4);
    REQUIRE(deck.slides[2].id == *between);
    REQUIRE(deck.slides[2].image == AssetId{"a-mid"});
    REQUIRE(deck.slides[3].id == CueId{"slide-1"});

    const auto after = insert_asset_slide(deck, deck.slides.size(), AssetId{"a-end"});
    REQUIRE(after);
    REQUIRE(deck.slides.size() == 5);
    REQUIRE(deck.slides.back().id == *after);
    REQUIRE(deck.slides.back().image == AssetId{"a-end"});
}

TEST_CASE("slide insertion rejects out-of-range indices without mutation and avoids id collisions") {
    Song song{SongId{"song-1"},
              "Title",
              std::nullopt,
              {Slide{CueId{"song-1-media-1"}, "Media", {}, BackgroundRef{AssetId{"a-0"}}}}};

    const auto invalid_song = insert_asset_slide(song, 2, BackgroundRef{AssetId{"a-1"}});
    REQUIRE_FALSE(invalid_song);
    REQUIRE(invalid_song.error().code == ErrorCode::invalid_input);
    REQUIRE(song.slides.size() == 1);

    const auto valid_song = insert_asset_slide(song, 1, BackgroundRef{AssetId{"a-1"}});
    REQUIRE(valid_song);
    REQUIRE(*valid_song != song.slides[0].id);

    SermonDeck deck{SermonId{"sermon-1"},
                    "Title",
                    {SermonSlide{CueId{"sermon-1-slide-1"}, AssetId{"a-0"}}}};
    const auto invalid_deck = insert_asset_slide(deck, 2, AssetId{"a-1"});
    REQUIRE_FALSE(invalid_deck);
    REQUIRE(invalid_deck.error().code == ErrorCode::invalid_input);
    REQUIRE(deck.slides.size() == 1);

    const auto valid_deck = insert_asset_slide(deck, 0, AssetId{"a-1"});
    REQUIRE(valid_deck);
    REQUIRE(*valid_deck != deck.slides[1].id);
}
