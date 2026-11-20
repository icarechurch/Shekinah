#include <catch2/catch_test_macros.hpp>

#include <QApplication>
#include <QScreen>

#include "shekinah/media/audience_output.hpp"

using namespace shekinah::domain;
using namespace shekinah::media;

namespace {

QApplication& ensure_qapplication() {
    static int argc = 1;
    static char application_name[] = "shekinah_media_tests";
    static char* argv[] = {application_name, nullptr};
    static QApplication application{argc, argv};
    return application;
}

}  // namespace

TEST_CASE("audience output supports movable and selected-screen fullscreen modes") {
    auto& application = ensure_qapplication();
    AudienceOutput output;
    output.show_movable();
    REQUIRE(output.mode() == OutputMode::movable);
    REQUIRE_FALSE(output.isFullScreen());

    REQUIRE(application.primaryScreen() != nullptr);
    output.show_fullscreen(application.primaryScreen());
    REQUIRE(output.mode() == OutputMode::fullscreen);
    REQUIRE(output.isFullScreen());

    output.show_movable();
    REQUIRE(output.mode() == OutputMode::movable);
}

TEST_CASE("audience output distinguishes clear, black, and live cue states") {
    ensure_qapplication();
    AudienceOutput output;
    output.present(ProgramState{ProgramMode::clear, std::nullopt});
    REQUIRE(output.program().mode == ProgramMode::clear);
    output.present(ProgramState{
        ProgramMode::cue,
        CueRef{ContentRef::song("song-a"), CueId{"chorus-1"}},
    });
    REQUIRE(output.program().live_cue->cue.value == "chorus-1");
    output.present(ProgramState{ProgramMode::black, std::nullopt});
    REQUIRE(output.program().mode == ProgramMode::black);
}

TEST_CASE("audience output exposes one video pause control") {
    ensure_qapplication();
    AudienceOutput output;
    output.set_video_paused(true);
    REQUIRE(output.video_paused());
    output.set_video_paused(false);
    REQUIRE_FALSE(output.video_paused());
}
