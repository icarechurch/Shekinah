#include <catch2/catch_test_macros.hpp>

#include <QApplication>
#include <QColor>
#include <QImage>
#include <QScreen>
#include <QTemporaryFile>

#include <algorithm>

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

TEST_CASE("audience output stores and clears live lyric text") {
    ensure_qapplication();
    AudienceOutput output;
    output.set_live_text(QStringLiteral("You are good\nAll the time"));

    REQUIRE(output.live_text() == QStringLiteral("You are good\nAll the time"));

    output.present(ProgramState{ProgramMode::clear, std::nullopt});

    REQUIRE(output.live_text().isEmpty());
}

TEST_CASE("audience output paints centered slide content without the grid checkerboard") {
    ensure_qapplication();
    AudienceOutput output;
    output.resize(320, 180);
    output.present(ProgramState{
        ProgramMode::cue,
        CueRef{ContentRef::song("song-a"), CueId{"slide-1"}},
    });
    output.set_live_text(QStringLiteral("Aming Ama"));

    QImage rendered{output.size(), QImage::Format_ARGB32_Premultiplied};
    rendered.fill(Qt::transparent);
    output.render(&rendered);

    REQUIRE(rendered.pixelColor(4, 4) == QColor(QStringLiteral("#182337")));
    REQUIRE(rendered.pixelColor(20, 4) == QColor(QStringLiteral("#182337")));
    REQUIRE(rendered.pixelColor(5, 80) == QColor(QStringLiteral("#182337")));

    bool has_center_content = false;
    for (int y = 30; y < 150 && !has_center_content; ++y) {
        for (int x = 20; x < 300; ++x) {
            if (rendered.pixelColor(x, y) != QColor(QStringLiteral("#182337"))) {
                has_center_content = true;
                break;
            }
        }
    }
    REQUIRE(has_center_content);
}

TEST_CASE("audience output scales slide text for a church-sized screen") {
    ensure_qapplication();
    AudienceOutput output;
    output.resize(1920, 1080);
    output.present(ProgramState{
        ProgramMode::cue,
        CueRef{ContentRef::song("song-a"), CueId{"slide-1"}},
    });
    output.set_live_text(QStringLiteral("GINAWA MO’NG LAHAT\nPAG-IBIG MO AY TAPAT AT WAGAS"));

    QImage rendered{output.size(), QImage::Format_ARGB32_Premultiplied};
    rendered.fill(Qt::transparent);
    output.render(&rendered);

    const QColor background{QStringLiteral("#182337")};
    int min_x = rendered.width();
    int max_x = -1;
    int min_y = rendered.height();
    int max_y = -1;
    for (int y = 0; y < rendered.height(); ++y) {
        for (int x = 0; x < rendered.width(); ++x) {
            if (rendered.pixelColor(x, y) == background) {
                continue;
            }
            min_x = std::min(min_x, x);
            max_x = std::max(max_x, x);
            min_y = std::min(min_y, y);
            max_y = std::max(max_y, y);
        }
    }

    REQUIRE(max_x - min_x + 1 > 500);
    REQUIRE(max_y - min_y + 1 > 70);
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
