#include <catch2/catch_test_macros.hpp>

#include "shekinah/domain/presentation_engine.hpp"

using namespace shekinah::domain;

TEST_CASE("selecting content never changes the current program") {
    PresentationEngine engine;
    engine.take_live(CueRef{ContentRef::song("song-a"), CueId{"chorus-2"}});
    engine.select_content(ContentRef::song("song-b"));

    REQUIRE(engine.program().mode == ProgramMode::cue);
    REQUIRE(engine.program().live_cue->cue.value == "chorus-2");
}

TEST_CASE("taking a cue live replaces the program cue") {
    PresentationEngine engine;
    engine.take_live(CueRef{ContentRef::song("song-a"), CueId{"bridge"}});

    REQUIRE(engine.program().mode == ProgramMode::cue);
    REQUIRE(engine.program().live_cue->cue.value == "bridge");
}

TEST_CASE("clear and black are explicit program modes") {
    PresentationEngine engine;
    engine.clear();
    REQUIRE(engine.program().mode == ProgramMode::clear);
    engine.black();
    REQUIRE(engine.program().mode == ProgramMode::black);
}
