#include <catch2/catch_test_macros.hpp>

#include "shekinah/application/cue_presenter.hpp"

#include <filesystem>

using namespace shekinah::application;
using namespace shekinah::domain;

TEST_CASE("song cue presentation preserves summaries and background ranges") {
    const Song song{
        SongId{"song"},
        "Goodness",
        BackgroundRef{AssetId{"default"}, FitMode::cover},
        {
            Slide{CueId{"verse"}, "Verse", {"One", "Two"}, std::nullopt},
            Slide{CueId{"chorus"}, "Chorus", {"Three"}, std::nullopt},
            Slide{CueId{"bridge"}, "Bridge", {}, std::nullopt},
            Slide{CueId{"tag"}, "Tag", {"Four"},
                  BackgroundRef{AssetId{"override"}, FitMode::contain}},
        },
    };

    const auto presented = build_song_cue_presentation(song, [](const AssetId& asset) {
        return std::filesystem::path{"media"} / asset.value;
    });

    REQUIRE(presented.title == "Goodness");
    REQUIRE(presented.cues.size() == 4);
    REQUIRE(presented.quick_cues.size() == 4);
    REQUIRE(presented.cues[0].summary == "One\nTwo");
    REQUIRE(presented.cues[0].background_path == std::filesystem::path{"media/default"});
    REQUIRE(presented.cues[0].background_fit == FitMode::cover);
    REQUIRE(presented.cues[1].background_path.empty());
    REQUIRE(presented.cues[2].background_path == std::filesystem::path{"media/default"});
    REQUIRE(presented.cues[3].background_path == std::filesystem::path{"media/override"});
    REQUIRE(presented.cues[3].background_fit == FitMode::contain);
    REQUIRE(presented.cues[0].footer_label == "Verse");
    REQUIRE(presented.cues[0].show_text);
    REQUIRE(presented.quick_cues[0].summary == "One\nTwo");
    REQUIRE(presented.quick_cues[0].number == 1);
}

TEST_CASE("sermon cue presentation creates numbered image slides") {
    const SermonDeck sermon{
        SermonId{"sermon"},
        "Hope",
        {
            SermonSlide{CueId{"slide-one"}, AssetId{"image-one"}},
            SermonSlide{CueId{"slide-two"}, AssetId{"image-two"}},
        },
    };

    const auto presented = build_sermon_cue_presentation(sermon, [](const AssetId& asset) {
        return std::filesystem::path{"media"} / asset.value;
    });

    REQUIRE(presented.title == "Hope");
    REQUIRE(presented.cues.size() == 2);
    REQUIRE(presented.cues[0].id == CueId{"slide-one"});
    REQUIRE(presented.cues[0].background_path == std::filesystem::path{"media/image-one"});
    REQUIRE(presented.cues[0].footer_label == "Slide 1");
    REQUIRE_FALSE(presented.cues[0].show_text);
    REQUIRE(presented.quick_cues[1].label == "Slide 2");
    REQUIRE(presented.quick_cues[1].summary.empty());
}
