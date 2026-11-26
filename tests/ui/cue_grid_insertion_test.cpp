#include <catch2/catch_test_macros.hpp>

#include <QApplication>
#include <QColor>
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
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

TEST_CASE("cue grid accepts asset insertion before first slide, between slides, and after last slide") {
    ensure_qapplication();
    CueGridWidget grid;
    grid.resize(900, 420);
    grid.set_cues({
        CueView{CueId{"cue-1"}, "Verse", "One", 1},
        CueView{CueId{"cue-2"}, "Chorus", "Two", 2},
        CueView{CueId{"cue-3"}, "Bridge", "Three", 3},
    });
    std::vector<std::pair<std::size_t, AssetId>> insertions;
    bool background_dropped = false;
    grid.set_asset_drop_handler([&](CueId, AssetId) { background_dropped = true; });
    grid.set_asset_insert_handler([&](std::size_t index, AssetId asset) {
        insertions.emplace_back(index, std::move(asset));
    });
    grid.show();
    QTest::qWait(30);

    auto* container = grid.findChild<QWidget*>(QStringLiteral("cue-grid-container"));
    auto* card1 = grid.findChild<QWidget*>(QStringLiteral("cue-cue-1"));
    auto* card2 = grid.findChild<QWidget*>(QStringLiteral("cue-cue-2"));
    auto* card3 = grid.findChild<QWidget*>(QStringLiteral("cue-cue-3"));
    REQUIRE(container != nullptr);
    REQUIRE(card1 != nullptr);
    REQUIRE(card2 != nullptr);
    REQUIRE(card3 != nullptr);

    auto* mime = shekinah::ui::make_asset_mime_data(AssetId{"asset-insert"});

    // 1. Before the first slide (left margin of container)
    const QPoint before_first{std::max(1, card1->x() / 2), card1->geometry().center().y()};
    const auto before_pos = test_support::drag_event_position(before_first);
    QDragEnterEvent enter_before{before_pos, Qt::CopyAction, mime, Qt::LeftButton, Qt::NoModifier};
    QApplication::sendEvent(container, &enter_before);
    REQUIRE(enter_before.isAccepted());
    REQUIRE(grid.property("cue-insert-index").toInt() == 0);
    QDropEvent drop_before{before_pos, Qt::CopyAction, mime, Qt::LeftButton, Qt::NoModifier};
    QApplication::sendEvent(container, &drop_before);
    REQUIRE(drop_before.isAccepted());
    REQUIRE(grid.property("cue-insert-index").toInt() == -1);

    // 2. Between slide 1 and slide 2 (gap in container)
    const QPoint between_1_2{(card1->geometry().right() + 1 + card2->x()) / 2,
                             card1->geometry().center().y()};
    const auto between_pos = test_support::drag_event_position(between_1_2);
    QDragEnterEvent enter_between{between_pos, Qt::CopyAction, mime, Qt::LeftButton, Qt::NoModifier};
    QApplication::sendEvent(container, &enter_between);
    REQUIRE(enter_between.isAccepted());
    REQUIRE(grid.property("cue-insert-index").toInt() == 1);
    QDropEvent drop_between{between_pos, Qt::CopyAction, mime, Qt::LeftButton, Qt::NoModifier};
    QApplication::sendEvent(container, &drop_between);
    REQUIRE(drop_between.isAccepted());

    // 3. Left edge hot-zone of slide 3 (inserts between slide 2 and slide 3)
    const auto edge_left_pos = test_support::drag_event_position(
        QPoint{2, card3->rect().center().y()});
    QDragEnterEvent enter_edge{edge_left_pos, Qt::CopyAction, mime, Qt::LeftButton, Qt::NoModifier};
    QApplication::sendEvent(card3, &enter_edge);
    REQUIRE(enter_edge.isAccepted());
    REQUIRE_FALSE(card3->property("cue-drop-target").toBool());
    REQUIRE(grid.property("cue-insert-index").toInt() == 2);
    QDropEvent drop_edge{edge_left_pos, Qt::CopyAction, mime, Qt::LeftButton, Qt::NoModifier};
    QApplication::sendEvent(card3, &drop_edge);
    REQUIRE(drop_edge.isAccepted());

    // 4. After the last slide (to the right of card 3)
    const QPoint after_last{card3->geometry().right() + 6, card3->geometry().center().y()};
    const auto after_pos = test_support::drag_event_position(after_last);
    QDragEnterEvent enter_after{after_pos, Qt::CopyAction, mime, Qt::LeftButton, Qt::NoModifier};
    QApplication::sendEvent(container, &enter_after);
    REQUIRE(enter_after.isAccepted());
    REQUIRE(grid.property("cue-insert-index").toInt() == 3);
    QDropEvent drop_after{after_pos, Qt::CopyAction, mime, Qt::LeftButton, Qt::NoModifier};
    QApplication::sendEvent(container, &drop_after);
    REQUIRE(drop_after.isAccepted());

    // 5. DragLeave clears indicator, unsupported MIME is rejected
    QDragEnterEvent enter_leave{before_pos, Qt::CopyAction, mime, Qt::LeftButton, Qt::NoModifier};
    QApplication::sendEvent(container, &enter_leave);
    REQUIRE(grid.property("cue-insert-index").toInt() == 0);
    QDragLeaveEvent leave;
    QApplication::sendEvent(container, &leave);
    REQUIRE(grid.property("cue-insert-index").toInt() == -1);

    QMimeData unsupported;
    QDropEvent bad_drop{before_pos, Qt::CopyAction, &unsupported, Qt::LeftButton, Qt::NoModifier};
    QApplication::sendEvent(container, &bad_drop);
    REQUIRE_FALSE(bad_drop.isAccepted());

    delete mime;

    REQUIRE_FALSE(background_dropped);
    REQUIRE(insertions.size() == 4);
    REQUIRE(insertions[0] == std::pair<std::size_t, AssetId>{0, AssetId{"asset-insert"}});
    REQUIRE(insertions[1] == std::pair<std::size_t, AssetId>{1, AssetId{"asset-insert"}});
    REQUIRE(insertions[2] == std::pair<std::size_t, AssetId>{2, AssetId{"asset-insert"}});
    REQUIRE(insertions[3] == std::pair<std::size_t, AssetId>{3, AssetId{"asset-insert"}});
}
TEST_CASE("cue grid handles row-wrap boundaries and single-column gaps for asset insertion") {
    ensure_qapplication();
    CueGridWidget grid;
    grid.resize(700, 600);
    grid.set_cues({
        CueView{CueId{"cue-1"}, "Verse", "One", 1},
        CueView{CueId{"cue-2"}, "Chorus", "Two", 2},
        CueView{CueId{"cue-3"}, "Bridge", "Three", 3},
        CueView{CueId{"cue-4"}, "Tag", "Four", 4},
    });
    std::vector<std::size_t> insertions;
    grid.set_asset_insert_handler([&](std::size_t index, AssetId) { insertions.push_back(index); });
    grid.show();
    QTest::qWait(30);

    auto* container = grid.findChild<QWidget*>(QStringLiteral("cue-grid-container"));
    auto* card3 = grid.findChild<QWidget*>(QStringLiteral("cue-cue-3"));
    auto* card4 = grid.findChild<QWidget*>(QStringLiteral("cue-cue-4"));
    REQUIRE(container != nullptr);
    REQUIRE(card3 != nullptr);
    REQUIRE(card4 != nullptr);
    REQUIRE(card4->y() > card3->y());

    auto* mime = shekinah::ui::make_asset_mime_data(AssetId{"asset-wrap"});
    // Left margin before card4 (first card of row 1) inserts at index 3 (between card3 and card4)
    const auto row1_left = test_support::drag_event_position(
        QPoint{std::max(1, card4->x() / 2), card4->geometry().center().y()});
    QDragEnterEvent enter_wrap{row1_left, Qt::CopyAction, mime, Qt::LeftButton, Qt::NoModifier};
    QApplication::sendEvent(container, &enter_wrap);
    REQUIRE(enter_wrap.isAccepted());
    REQUIRE(grid.property("cue-insert-index").toInt() == 3);
    QDropEvent drop_wrap{row1_left, Qt::CopyAction, mime, Qt::LeftButton, Qt::NoModifier};
    QApplication::sendEvent(container, &drop_wrap);
    REQUIRE(drop_wrap.isAccepted());

    // Resize narrow to force 1 column and drop in the vertical gap between card1 and card2
    grid.resize(280, 720);
    QTest::qWait(30);
    auto* card1 = grid.findChild<QWidget*>(QStringLiteral("cue-cue-1"));
    auto* card2 = grid.findChild<QWidget*>(QStringLiteral("cue-cue-2"));
    REQUIRE(card1 != nullptr);
    REQUIRE(card2 != nullptr);
    REQUIRE(card2->y() > card1->geometry().bottom());

    const auto vertical_gap = test_support::drag_event_position(
        QPoint{card1->geometry().center().x(),
               (card1->geometry().bottom() + 1 + card2->y()) / 2});
    QDragEnterEvent enter_single_col{
        vertical_gap, Qt::CopyAction, mime, Qt::LeftButton, Qt::NoModifier};
    QApplication::sendEvent(container, &enter_single_col);
    REQUIRE(enter_single_col.isAccepted());
    REQUIRE(grid.property("cue-insert-index").toInt() == 1);
    QDropEvent drop_single_col{
        vertical_gap, Qt::CopyAction, mime, Qt::LeftButton, Qt::NoModifier};
    QApplication::sendEvent(container, &drop_single_col);
    REQUIRE(drop_single_col.isAccepted());

    delete mime;
    REQUIRE(insertions == std::vector<std::size_t>{3, 1});
}

