#include <catch2/catch_test_macros.hpp>

#include "shekinah/domain/model.hpp"

using namespace shekinah::domain;

TEST_CASE("a song stores ordered lyric slides and a default background") {
    Song song{SongId{"song-a"}, "Goodness of God", std::nullopt, {}};
    song.slides.push_back(Slide{CueId{"verse-1"}, "Verse 1", {"I've loved You"}, std::nullopt});
    song.default_background = BackgroundRef{AssetId{"video-a"}, FitMode::cover};

    REQUIRE(song.slides.front().section == "Verse 1");
    REQUIRE(song.default_background->asset.value == "video-a");
}

TEST_CASE("service order entries reference reusable content") {
    ServiceOrderItem item{ServiceOrderItemId{"item-a"}, ContentRef::song("song-a"), 7};
    REQUIRE(item.content.kind == ContentKind::song);
    REQUIRE(item.ordinal == 7);
}
