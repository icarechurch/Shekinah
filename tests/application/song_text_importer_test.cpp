#include <catch2/catch_test_macros.hpp>

#include "shekinah/application/song_text_importer.hpp"

using namespace shekinah::application;
using namespace shekinah::domain;

TEST_CASE("song text uses section headers and blank lines as slide boundaries") {
    const auto song = SongTextImporter::parse(
        "# Goodness\n[Verse 1]\nLine one\n\nLine two\n[Chorus]\nSing");

    REQUIRE(song);
    REQUIRE(song->title == "Goodness");
    REQUIRE(song->slides.size() == 3);
    REQUIRE(song->slides[0].section == "Verse 1");
    REQUIRE(song->slides[0].lines == std::vector<std::string>{"Line one"});
    REQUIRE(song->slides[2].section == "Chorus");
}

TEST_CASE("song text rejects an empty document") {
    const auto result = SongTextImporter::parse("  \n\n");

    REQUIRE_FALSE(result);
    REQUIRE(result.error().code == ErrorCode::invalid_input);
}
