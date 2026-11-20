#include <catch2/catch_test_macros.hpp>

#include "shekinah/application/song_reflow.hpp"

using namespace shekinah::application;

TEST_CASE("inserting then removing a Reflow break splits then merges a Song") {
    SongReflowDocument document{"First line\nSecond line", {}};

    REQUIRE(document.insert_slide_break(10));
    REQUIRE(document.to_slides().size() == 2);
    REQUIRE(document.remove_slide_break(10));
    REQUIRE(document.to_slides().size() == 1);
}

TEST_CASE("Reflow rejects a break inside a UTF-8 code point") {
    SongReflowDocument document{"Café", {}};

    const auto result = document.insert_slide_break(4);

    REQUIRE_FALSE(result);
    REQUIRE(result.error().code == shekinah::domain::ErrorCode::invalid_input);
}
