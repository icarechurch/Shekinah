#include <catch2/catch_test_macros.hpp>

#include <QApplication>
#include <QColor>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QImage>
#include <QMimeData>
#include <QPushButton>
#include <QScrollArea>
#include <QScrollBar>
#include <QTemporaryDir>
#include <QTest>

#include "qt_drag_event_position.hpp"
#include "shekinah/ui/cue_grid_widget.hpp"
#include "shekinah/ui/support/asset_drag.hpp"

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

TEST_CASE("cue grid scrolls vertically without horizontal scrolling") {
    ensure_qapplication();
    CueGridWidget grid;
    grid.resize(480, 240);
    std::vector<CueView> cues;
    for (int number = 1; number <= 9; ++number) {
        cues.emplace_back(
            CueId{"cue-" + std::to_string(number)}, "Verse", "Line", number);
    }
    grid.set_cues(std::move(cues));
    grid.show();
    QTest::qWait(20);

    auto* scroll = grid.findChild<QScrollArea*>(QStringLiteral("cue-grid-scroll"));
    REQUIRE(scroll != nullptr);
    REQUIRE(scroll->horizontalScrollBarPolicy() == Qt::ScrollBarAlwaysOff);
    REQUIRE(scroll->verticalScrollBarPolicy() == Qt::ScrollBarAsNeeded);
    REQUIRE(scroll->verticalScrollBar()->isVisible());
    REQUIRE_FALSE(scroll->horizontalScrollBar()->isVisible());
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

TEST_CASE("cue card accepts an asset drop and reports its cue") {
    ensure_qapplication();
    CueGridWidget grid;
    grid.resize(700, 500);
    grid.set_cues({CueView{CueId{"cue-1"}, "Verse", "Line", 1}});
    CueId target{"unset"};
    AssetId asset{"unset"};
    grid.set_asset_drop_handler([&](CueId cue, AssetId dropped) {
        target = std::move(cue);
        asset = std::move(dropped);
    });
    grid.show();
    QTest::qWait(20);

    auto* card = grid.findChild<QWidget*>(QStringLiteral("cue-cue-1"));
    REQUIRE(card != nullptr);
    auto* mime = shekinah::ui::make_asset_mime_data(AssetId{"asset-1"});
    const auto position = test_support::drag_event_position(card->rect().center());
    QDragEnterEvent enter{position, Qt::CopyAction, mime, Qt::LeftButton, Qt::NoModifier};
    QApplication::sendEvent(card, &enter);
    REQUIRE(enter.isAccepted());
    QDragMoveEvent move{position, Qt::CopyAction, mime, Qt::LeftButton, Qt::NoModifier};
    QApplication::sendEvent(card, &move);
    REQUIRE(move.isAccepted());
    QDropEvent event{position, Qt::CopyAction, mime, Qt::LeftButton, Qt::NoModifier};
    QApplication::sendEvent(card, &event);

    REQUIRE(event.isAccepted());
    REQUIRE(target == CueId{"cue-1"});
    REQUIRE(asset == AssetId{"asset-1"});
    delete mime;
}

TEST_CASE("cue card ignores unsupported asset drops") {
    ensure_qapplication();
    CueGridWidget grid;
    grid.resize(700, 500);
    grid.set_cues({CueView{CueId{"cue-1"}, "Verse", "Line", 1}});
    bool callback_called = false;
    grid.set_asset_drop_handler([&](CueId, AssetId) { callback_called = true; });
    grid.show();
    QTest::qWait(20);

    auto* card = grid.findChild<QWidget*>(QStringLiteral("cue-cue-1"));
    REQUIRE(card != nullptr);
    QMimeData unsupported;
    const auto position = test_support::drag_event_position(card->rect().center());
    QDragEnterEvent enter{position, Qt::CopyAction, &unsupported, Qt::LeftButton, Qt::NoModifier};
    QApplication::sendEvent(card, &enter);
    REQUIRE_FALSE(enter.isAccepted());
    QDropEvent event{position, Qt::CopyAction, &unsupported, Qt::LeftButton, Qt::NoModifier};
    QApplication::sendEvent(card, &event);

    REQUIRE_FALSE(event.isAccepted());
    REQUIRE_FALSE(callback_called);
}

TEST_CASE("cue card marks a valid asset drag target and clears it after drop") {
    ensure_qapplication();
    CueGridWidget grid;
    grid.resize(700, 500);
    grid.set_cues({CueView{CueId{"cue-1"}, "Verse", "Line", 1}});
    grid.set_asset_drop_handler([](CueId, AssetId) {});
    grid.show();
    QTest::qWait(20);

    auto* card = grid.findChild<QWidget*>(QStringLiteral("cue-cue-1"));
    REQUIRE(card != nullptr);
    REQUIRE_FALSE(card->property("cue-drop-target").toBool());
    auto* mime = shekinah::ui::make_asset_mime_data(AssetId{"asset-1"});
    const auto position = test_support::drag_event_position(card->rect().center());
    QDragEnterEvent enter{position, Qt::CopyAction, mime, Qt::LeftButton, Qt::NoModifier};
    QApplication::sendEvent(card, &enter);

    REQUIRE(enter.isAccepted());
    REQUIRE(card->property("cue-drop-target").toBool());
    QDropEvent drop{position, Qt::CopyAction, mime, Qt::LeftButton, Qt::NoModifier};
    QApplication::sendEvent(card, &drop);

    REQUIRE(drop.isAccepted());
    REQUIRE_FALSE(card->property("cue-drop-target").toBool());
    delete mime;
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

TEST_CASE("cue card paints a usable background image") {
    ensure_qapplication();
    QTemporaryDir directory;
    REQUIRE(directory.isValid());
    const auto path = directory.filePath(QStringLiteral("background.png"));
    QImage background{40, 20, QImage::Format_RGB32};
    background.fill(Qt::red);
    REQUIRE(background.save(path));

    CueGridWidget grid;
    grid.resize(320, 240);
    grid.set_cues({CueView{
        CueId{"image"},
        "Verse",
        "You are here",
        1,
        path,
        FitMode::stretch,
        QStringLiteral("Verse"),
        true,
    }});
    auto* card = grid.findChild<QWidget*>(QStringLiteral("cue-image"));
    REQUIRE(card != nullptr);
    card->resize(320, card->heightForWidth(320));

    QImage image(card->size(), QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);
    card->render(&image);

    REQUIRE(image.pixelColor(12, 12).red() > 200);
    REQUIRE(image.pixelColor(12, 12).green() < 80);
}

TEST_CASE("cue card omits decorative lyric rules") {
    ensure_qapplication();
    QTemporaryDir directory;
    REQUIRE(directory.isValid());
    const auto path = directory.filePath(QStringLiteral("background.png"));
    QImage background{40, 20, QImage::Format_RGB32};
    background.fill(Qt::black);
    REQUIRE(background.save(path));

    CueGridWidget grid;
    grid.resize(320, 240);
    grid.set_cues({CueView{
        CueId{"no-rules"},
        "Verse",
        "Aming Ama",
        1,
        path,
        FitMode::stretch,
        QStringLiteral("Verse"),
        true,
    }});
    auto* card = grid.findChild<QWidget*>(QStringLiteral("cue-no-rules"));
    REQUIRE(card != nullptr);
    card->resize(320, card->heightForWidth(320));

    QImage image(card->size(), QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);
    card->render(&image);

    const auto preview = card->rect().adjusted(2, 2, -2, -34);
    REQUIRE(image.pixelColor(preview.center().x(), preview.center().y() - 31) == QColor(Qt::black));
    REQUIRE(image.pixelColor(preview.center().x(), preview.center().y() + 31) == QColor(Qt::black));
}

TEST_CASE("cue card falls back to checkerboard for an unreadable background") {
    ensure_qapplication();
    CueGridWidget grid;
    grid.resize(320, 240);
    grid.set_cues({CueView{
        CueId{"missing"},
        "Verse",
        "You are here",
        1,
        QStringLiteral("C:/does-not-exist/shekinah-background.png"),
        FitMode::cover,
        QStringLiteral("Verse"),
        true,
    }});
    auto* card = grid.findChild<QWidget*>(QStringLiteral("cue-missing"));
    REQUIRE(card != nullptr);
    card->resize(320, card->heightForWidth(320));

    QImage image(card->size(), QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);
    card->render(&image);

    REQUIRE(image.pixelColor(12, 12) != image.pixelColor(24, 12));
}

TEST_CASE("right clicking a cue dispatches its context handler") {
    ensure_qapplication();
    CueGridWidget grid;
    grid.set_cues({CueView{CueId{"context"}, "Verse", "Line", 1}});
    CueId requested;
    QPoint position;
    grid.set_context_handler([&](CueId cue, QPoint global_position) {
        requested = std::move(cue);
        position = global_position;
    });
    grid.show();
    QTest::qWait(20);

    auto* card = grid.findChild<QWidget*>(QStringLiteral("cue-context"));
    REQUIRE(card != nullptr);
    QTest::mouseClick(card, Qt::RightButton, Qt::NoModifier, card->rect().center());

    REQUIRE(requested.value == "context");
    REQUIRE(position.isNull() == false);
}

TEST_CASE("clearing the live cue removes every card highlight") {
    ensure_qapplication();
    CueGridWidget grid;
    grid.set_cues({
        CueView{CueId{"one"}, "Verse", "Line", 1},
        CueView{CueId{"two"}, "Chorus", "Line", 2},
    });
    grid.set_live_cue(CueId{"two"});
    grid.set_live_cue(std::nullopt);

    REQUIRE_FALSE(grid.findChild<QWidget*>(QStringLiteral("cue-one"))->property("cue-live").toBool());
    REQUIRE_FALSE(grid.findChild<QWidget*>(QStringLiteral("cue-two"))->property("cue-live").toBool());
}

TEST_CASE("cue card footer colors follow section names") {
    REQUIRE(CueGridWidget::footer_color_for_section("Verse 1") == QColor("#0b80ba"));
    REQUIRE(CueGridWidget::footer_color_for_section("Chorus 1") == QColor("#cc1f58"));
    REQUIRE(CueGridWidget::footer_color_for_section("Bridge") == QColor("#8336c7"));
    REQUIRE(CueGridWidget::footer_color_for_section("Tag") == QColor("#cf3a32"));
    REQUIRE(CueGridWidget::footer_color_for_section("Intro") == QColor("#0b80ba"));
}

TEST_CASE("cue card exposes numbered footer metadata") {
    ensure_qapplication();
    CueGridWidget grid;
    grid.set_cues({CueView{
        CueId{"footer"},
        "Chorus 1",
        "You are here",
        4,
        {},
        FitMode::cover,
        QStringLiteral("Chorus 1"),
        true,
    }});
    auto* card = grid.findChild<QWidget*>(QStringLiteral("cue-footer"));

    REQUIRE(card != nullptr);
    REQUIRE(card->property("cue-number").toInt() == 4);
    REQUIRE(card->property("cue-footer-label").toString() == QStringLiteral("Chorus 1"));
    REQUIRE(card->property("cue-section-color").value<QColor>() == QColor("#cc1f58"));
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
