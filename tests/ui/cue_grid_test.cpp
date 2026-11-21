#include <catch2/catch_test_macros.hpp>

#include <QApplication>
#include <QImage>
#include <QPushButton>
#include <QTest>

#include "shekinah/ui/cue_grid_widget.hpp"

using namespace shekinah::domain;
using namespace shekinah::ui;

namespace {

QApplication& ensure_qapplication() {
    static int argc = 1;
    static char application_name[] = "shekinah_ui_tests";
    static char* argv[] = {application_name, nullptr};
    static QApplication application{argc, argv};
    return application;
}

}  // namespace

TEST_CASE("cue grid follows the approved automatic column policy") {
    REQUIRE(CueGridWidget::column_count_for_geometry(700, 1366) == 3);
    REQUIRE(CueGridWidget::column_count_for_geometry(1000, 1920) == 4);
    REQUIRE(CueGridWidget::column_count_for_geometry(1500, 3200) == 6);
    REQUIRE(CueGridWidget::column_count_for_geometry(250, 1366) == 1);
    REQUIRE(CueGridWidget::column_count_for_geometry(450, 1366) == 2);
}

TEST_CASE("cue view carries card preview metadata") {
    CueView cue{
        CueId{"verse-1"},
        "Verse 1",
        "You are good",
        3,
        QStringLiteral("C:/assets/verse.png"),
        FitMode::cover,
        QStringLiteral("Verse 1"),
        true,
    };

    REQUIRE(cue.number == 3);
    REQUIRE(cue.background_path == QStringLiteral("C:/assets/verse.png"));
    REQUIRE(cue.background_fit == FitMode::cover);
    REQUIRE(cue.footer_label == QStringLiteral("Verse 1"));
    REQUIRE(cue.show_text);
}

TEST_CASE("left clicking a cue dispatches the cue without changing grid ownership") {
    ensure_qapplication();
    CueGridWidget grid;
    grid.resize(700, 500);
    grid.set_cues({
        CueView{CueId{"verse-1"}, "Verse 1", "One"},
        CueView{CueId{"chorus-1"}, "Chorus", "Two"},
    });
    CueId requested;
    grid.set_take_live_handler([&requested](CueId cue) { requested = std::move(cue); });
    grid.show();
    QTest::qWait(20);

    const auto rectangle = grid.visual_rect(CueId{"chorus-1"});
    REQUIRE(rectangle.isValid());
    auto* target = grid.findChild<QWidget*>(QStringLiteral("cue-chorus-1"));
    REQUIRE(target != nullptr);
    QTest::mouseClick(target, Qt::LeftButton, Qt::NoModifier, target->rect().center());
    REQUIRE(requested.value == "chorus-1");
}

TEST_CASE("cue cards show section and lyric summary text") {
    ensure_qapplication();
    CueGridWidget grid;
    grid.set_cues({CueView{CueId{"verse-1"}, "Verse 1", "You are good"}});
    auto* card = grid.findChild<QWidget*>(QStringLiteral("cue-verse-1"));

    REQUIRE(card != nullptr);
    REQUIRE(card->property("cue-section").toString() == QStringLiteral("Verse 1"));
    REQUIRE(card->property("cue-summary").toString() == QStringLiteral("You are good"));
}

TEST_CASE("cue card reserves a 16 by 9 preview and footer") {
    ensure_qapplication();
    CueGridWidget grid;
    grid.set_cues({CueView{CueId{"one"}, "Verse", "Line", 1}});
    auto* card = grid.findChild<QWidget*>(QStringLiteral("cue-one"));

    REQUIRE(card != nullptr);
    REQUIRE(card->property("cue-live").toBool() == false);
    REQUIRE(card->heightForWidth(320) >= 320 * 9 / 16);
}

TEST_CASE("cue grid marks only the matching card live") {
    ensure_qapplication();
    CueGridWidget grid;
    grid.set_cues({
        CueView{CueId{"one"}, "Verse", "Line", 1},
        CueView{CueId{"two"}, "Chorus", "Line", 2},
    });
    grid.set_live_cue(CueId{"two"});

    REQUIRE(grid.findChild<QWidget*>(QStringLiteral("cue-one"))->property("cue-live").toBool() == false);
    REQUIRE(grid.findChild<QWidget*>(QStringLiteral("cue-two"))->property("cue-live").toBool() == true);
}

TEST_CASE("cue card paints checkerboard when no background is assigned") {
    ensure_qapplication();
    CueGridWidget grid;
    grid.resize(320, 240);
    grid.set_cues({CueView{CueId{"empty"}, "Verse", "You are here", 1}});
    auto* card = grid.findChild<QWidget*>(QStringLiteral("cue-empty"));
    REQUIRE(card != nullptr);
    card->resize(320, card->heightForWidth(320));

    QImage image(card->size(), QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);
    card->render(&image);

    REQUIRE(image.pixelColor(12, 12) != image.pixelColor(24, 12));
}

TEST_CASE("empty Presentation view exposes authoring actions") {
    ensure_qapplication();
    CueGridWidget grid;
    int new_song = 0;
    int import_song = 0;
    int import_sermon = 0;
    int import_asset = 0;
    grid.set_authoring_handlers(
        [&] { ++new_song; },
        [&] { ++import_song; },
        [&] { ++import_sermon; },
        [&] { ++import_asset; });
    grid.show();

    REQUIRE(grid.findChild<QWidget*>(QStringLiteral("presentation-empty-state")) != nullptr);
    grid.findChild<QPushButton*>(QStringLiteral("presentation-new-song"))->click();
    grid.findChild<QPushButton*>(QStringLiteral("presentation-import-song"))->click();
    grid.findChild<QPushButton*>(QStringLiteral("presentation-import-sermon"))->click();
    grid.findChild<QPushButton*>(QStringLiteral("presentation-import-asset"))->click();

    REQUIRE(new_song == 1);
    REQUIRE(import_song == 1);
    REQUIRE(import_sermon == 1);
    REQUIRE(import_asset == 1);
}
