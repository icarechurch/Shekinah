#include <catch2/catch_test_macros.hpp>

#include <QApplication>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QListWidget>
#include <QMimeData>
#include <QPushButton>
#include <QSplitter>
#include <QTest>

#include "qt_drag_event_position.hpp"
#include "shekinah/ui/cue_grid_widget.hpp"
#include "shekinah/ui/operator_page.hpp"
#include "shekinah/ui/program_status_widget.hpp"
#include "shekinah/ui/support/content_drag.hpp"
#include "shekinah/ui/service_order_widget.hpp"
#include "shekinah/ui/support/asset_drag.hpp"

using namespace shekinah::ui;
using namespace shekinah::domain;

namespace {

QApplication& ensure_qapplication() {
    static int argc = 1;
    static char application_name[] = "shekinah_operator_tests";
    static char* argv[] = {application_name, nullptr};
    static QApplication application{argc, argv};
    return application;
}

}  // namespace

TEST_CASE("operator page places global controls below Program Status") {
    ensure_qapplication();
    OperatorPage page;
    page.resize(1280, 720);
    page.show();

    REQUIRE(page.program_status() != nullptr);
    REQUIRE(page.program_status()->global_controls_below_status());
    REQUIRE(page.findChild<QWidget*>(QStringLiteral("preview")) == nullptr);
    REQUIRE(page.findChild<QWidget*>(QStringLiteral("global-controls")) != nullptr);
}

TEST_CASE("operator page exposes Service Order, Cue Grid, and Program Status") {
    ensure_qapplication();
    OperatorPage page;
    REQUIRE(page.service_order_widget() != nullptr);
    REQUIRE(page.cue_grid() != nullptr);
    REQUIRE(page.program_status() != nullptr);
    page.set_service_order({ServiceOrderView{
        ServiceOrderItem{ServiceOrderItemId{"item-1"}, ContentRef::song("song-a"), 0},
        "Song A",
    }});
    REQUIRE(page.service_order_widget()->count() == 1);
}

TEST_CASE("operator workspace keeps the Presentation cue surface visible on the right") {
    ensure_qapplication();
    OperatorPage page;
    page.resize(1280, 720);
    page.show();
    QTest::qWait(20);

    auto* workspace = page.findChild<QSplitter*>(QStringLiteral("operator-workspace"));
    REQUIRE(workspace != nullptr);
    REQUIRE(workspace->count() == 3);
    REQUIRE(page.service_order_widget()->isVisible());
    REQUIRE(page.cue_grid()->isVisible());
    REQUIRE(page.program_status()->isVisible());
    REQUIRE(page.cue_grid()->width() > 0);
    REQUIRE(page.program_status()->width() > 0);
    REQUIRE(workspace->widget(2) == page.program_status());
    REQUIRE(page.cue_grid()->findChild<QWidget*>(QStringLiteral("presentation-empty-state")) != nullptr);
}

TEST_CASE("operator page forwards service-order selection and cue actions") {
    ensure_qapplication();
    OperatorPage page;
    page.set_service_order({ServiceOrderView{
        ServiceOrderItem{ServiceOrderItemId{"item-1"}, ContentRef::song("song-a"), 0},
        "Song A",
    }});
    ServiceOrderItemId selected;
    CueId live;
    page.set_service_order_selection_handler([&selected](ServiceOrderItemId id) {
        selected = std::move(id);
    });
    page.set_take_live_handler([&live](CueId cue) { live = std::move(cue); });

    page.service_order_widget()->select_row(0);
    page.cue_grid()->set_cues({CueView{CueId{"verse-1"}, "Verse", "One"}});
    page.show();
    QTest::qWait(20);
    auto* cue = page.cue_grid()->findChild<QWidget*>(QStringLiteral("cue-verse-1"));
    REQUIRE(cue != nullptr);
    QTest::mouseClick(cue, Qt::LeftButton, Qt::NoModifier, cue->rect().center());

    REQUIRE(selected == ServiceOrderItemId{"item-1"});
    REQUIRE(live == CueId{"verse-1"});
}

TEST_CASE("operator page forwards both asset drop surfaces") {
    ensure_qapplication();
    OperatorPage page;
    page.set_service_order({ServiceOrderView{
        ServiceOrderItem{ServiceOrderItemId{"item-1"}, ContentRef::song("song-a"), 0},
        "Song A",
    }});
    page.cue_grid()->set_cues({CueView{CueId{"cue-1"}, "Verse", "One"}});
    ContentRef dropped_content = ContentRef::sermon("unset");
    CueId dropped_cue{"unset"};
    AssetId dropped_asset{"unset"};
    page.set_service_order_asset_drop_handler([&](ContentRef content, AssetId asset) {
        dropped_content = std::move(content);
        dropped_asset = std::move(asset);
    });
    page.set_cue_asset_drop_handler([&](CueId cue, AssetId asset) {
        dropped_cue = std::move(cue);
        dropped_asset = std::move(asset);
    });
    page.show();
    QTest::qWait(20);

    auto* list = page.service_order_widget()->findChild<QListWidget*>(
        QStringLiteral("service-order-list"));
    REQUIRE(list != nullptr);
    auto* order_mime = shekinah::ui::make_asset_mime_data(AssetId{"asset-order"});
    const auto order_position = test_support::drag_event_position(
        list->visualItemRect(list->item(0)).center());
    QDragEnterEvent order_enter{
        order_position, Qt::CopyAction, order_mime, Qt::LeftButton, Qt::NoModifier};
    QApplication::sendEvent(list->viewport(), &order_enter);
    QDragMoveEvent order_move{
        order_position, Qt::CopyAction, order_mime, Qt::LeftButton, Qt::NoModifier};
    QApplication::sendEvent(list->viewport(), &order_move);
    QDropEvent order_drop{
        order_position, Qt::CopyAction, order_mime, Qt::LeftButton, Qt::NoModifier};
    QApplication::sendEvent(list->viewport(), &order_drop);
    delete order_mime;

    auto* card = page.cue_grid()->findChild<QWidget*>(QStringLiteral("cue-cue-1"));
    REQUIRE(card != nullptr);
    auto* cue_mime = shekinah::ui::make_asset_mime_data(AssetId{"asset-cue"});
    const auto cue_position = test_support::drag_event_position(card->rect().center());
    QDragEnterEvent cue_enter{cue_position, Qt::CopyAction, cue_mime, Qt::LeftButton, Qt::NoModifier};
    QApplication::sendEvent(card, &cue_enter);
    QDragMoveEvent cue_move{cue_position, Qt::CopyAction, cue_mime, Qt::LeftButton, Qt::NoModifier};
    QApplication::sendEvent(card, &cue_move);
    QDropEvent cue_drop{cue_position, Qt::CopyAction, cue_mime, Qt::LeftButton, Qt::NoModifier};
    QApplication::sendEvent(card, &cue_drop);
    delete cue_mime;

    REQUIRE(dropped_content == ContentRef::song("song-a"));
    REQUIRE(dropped_cue == CueId{"cue-1"});
    REQUIRE(dropped_asset == AssetId{"asset-cue"});
}

TEST_CASE("operator page forwards reusable content drops and removal") {
    ensure_qapplication();
    OperatorPage page;
    page.set_service_order({ServiceOrderView{
        ServiceOrderItem{ServiceOrderItemId{"item-1"}, ContentRef::song("song-1"), 0},
        QStringLiteral("Song")}});
    ContentRef dropped = ContentRef::sermon("unset");
    ServiceOrderItemId removed{"unset"};
    page.set_service_order_content_drop_handler([&](ContentRef content) { dropped = std::move(content); });
    page.set_service_order_remove_handler([&](ServiceOrderItemId item) { removed = std::move(item); });
    page.show();
    QTest::qWait(20);

    auto* list = page.service_order_widget()->findChild<QListWidget*>(
        QStringLiteral("service-order-list"));
    REQUIRE(list != nullptr);
    auto* mime = shekinah::ui::make_content_mime_data(ContentRef::song("song-2"));
    const auto position = test_support::drag_event_position(
        list->visualItemRect(list->item(0)).center());
    QDragEnterEvent enter{position, Qt::CopyAction, mime, Qt::LeftButton, Qt::NoModifier};
    QApplication::sendEvent(list->viewport(), &enter);
    QDragMoveEvent move{position, Qt::CopyAction, mime, Qt::LeftButton, Qt::NoModifier};
    QApplication::sendEvent(list->viewport(), &move);
    QDropEvent drop{position, Qt::CopyAction, mime, Qt::LeftButton, Qt::NoModifier};
    QApplication::sendEvent(list->viewport(), &drop);
    delete mime;

    REQUIRE(drop.isAccepted());
    REQUIRE(dropped == ContentRef::song("song-2"));
    page.service_order_widget()->select_row(0);
    page.service_order_widget()->findChild<QPushButton*>(QStringLiteral("service-order-remove"))->click();
    REQUIRE(removed == ServiceOrderItemId{"item-1"});
}
