#include <catch2/catch_test_macros.hpp>

#include "shekinah/media/low_end_profile.hpp"

using namespace shekinah::domain;
using namespace shekinah::media;

TEST_CASE("N4000 profile rejects nonaccelerated video before output changes") {
    LowEndCompatibilityProfile profile;
    const auto result = profile.preflight(VideoInfo{1920, 1080, 30, false});

    REQUIRE_FALSE(result);
    REQUIRE(result.error().code == ErrorCode::unsupported_media);
}

TEST_CASE("N4000 profile accepts one accelerated 1080p30 video") {
    LowEndCompatibilityProfile profile;
    REQUIRE(profile.preflight(VideoInfo{1920, 1080, 30, true}));
}

TEST_CASE("N4000 profile rejects video above its resolution or frame-rate ceiling") {
    LowEndCompatibilityProfile profile;
    REQUIRE_FALSE(profile.preflight(VideoInfo{2560, 1440, 30, true}));
    REQUIRE_FALSE(profile.preflight(VideoInfo{1920, 1080, 60, true}));
}
