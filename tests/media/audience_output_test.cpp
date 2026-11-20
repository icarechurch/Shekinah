#include <catch2/catch_test_macros.hpp>

#include <QApplication>
#include <QImage>
#include <QScreen>
#include <QTemporaryFile>

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

TEST_CASE("audience output decodes one bounded slide image and releases it") {
    ensure_qapplication();
    QTemporaryFile temporary;
    REQUIRE(temporary.open());
    QImage image{4000, 2000, QImage::Format_RGB32};
    image.fill(Qt::red);
    REQUIRE(image.save(temporary.fileName(), "PNG"));

    AudienceOutput output;
    REQUIRE(output.load_slide_image(temporary.fileName().toStdString()));
    REQUIRE(output.slide_image_loaded());
    output.clear_slide_image();
    REQUIRE_FALSE(output.slide_image_loaded());
}

TEST_CASE("audience output loads one loop video and releases it on clear") {
    ensure_qapplication();
    QTemporaryFile temporary;
    REQUIRE(temporary.open());
    temporary.write("not-a-real-video");
    temporary.flush();

    AudienceOutput output;
    REQUIRE(output.load_loop_video(temporary.fileName().toStdString()));
    REQUIRE(output.loop_video_loaded());
    output.clear_loop_video();
    REQUIRE_FALSE(output.loop_video_loaded());
}

TEST_CASE("audience output rejects a missing loop-video path") {
    ensure_qapplication();
    AudienceOutput output;
    const auto result = output.load_loop_video("C:/does-not-exist/shekinah.mp4");
    REQUIRE_FALSE(result);
    REQUIRE(result.error().code == ErrorCode::io_failure);
}
