#include <catch2/catch_test_macros.hpp>

#include <QApplication>
#include <QDropEvent>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QListWidget>
#include <QMimeData>
#include <QPushButton>
#include <QTest>

#include "shekinah/ui/service_order_widget.hpp"
#include "shekinah/ui/support/asset_drag.hpp"
#include "shekinah/ui/support/content_drag.hpp"

using namespace shekinah::domain;
using namespace shekinah::ui;

namespace {

QApplication& ensure_qapplication() {
    static int argc = 1;
    static char application_name[] = "shekinah_service_order_tests";
    static char* argv[] = {application_name, nullptr};
    static QApplication application{argc, argv};
    return application;
}

}  // namespace

TEST_CASE("service order widget presents reusable references and reports selection") {
    ensure_qapplication();
    ServiceOrderWidget widget;
    widget.set_items({
        ServiceOrderView{ServiceOrderItem{ServiceOrderItemId{"item-1"}, ContentRef::song("song-a"), 0}, "Song A"},
        ServiceOrderView{ServiceOrderItem{ServiceOrderItemId{"item-2"}, ContentRef::sermon("sermon-a"), 1}, "Sermon A"},
    });
    ServiceOrderItemId selected;
    widget.set_selection_handler([&selected](ServiceOrderItemId id) { selected = std::move(id); });
    widget.show();
    widget.select_row(1);
    QTest::qWait(20);

    REQUIRE(widget.count() == 2);
    REQUIRE(selected.value == "item-2");
}

TEST_CASE("service order widget reports requested moves from its controls") {
    ensure_qapplication();
    ServiceOrderWidget widget;
    widget.set_items({
        ServiceOrderView{ServiceOrderItem{ServiceOrderItemId{"item-1"}, ContentRef::song("song-a"), 0}, "Song A"},
        ServiceOrderView{ServiceOrderItem{ServiceOrderItemId{"item-2"}, ContentRef::sermon("sermon-a"), 1}, "Sermon A"},
    });
    ServiceOrderItemId moved;
    std::int64_t target = -1;
    widget.set_move_handler([&](ServiceOrderItemId id, std::int64_t ordinal) {
        moved = std::move(id);
        target = ordinal;
    });
    widget.show();
    widget.select_row(1);
    QTest::qWait(20);

    auto* move_up = widget.findChild<QPushButton*>(QStringLiteral("service-order-move-up"));
    REQUIRE(move_up != nullptr);
    move_up->click();

    REQUIRE(moved.value == "item-2");
    REQUIRE(target == 0);
}

TEST_CASE("service order reports an asset dropped on a Song row") {
    ensure_qapplication();
    ServiceOrderWidget widget;
    widget.set_items({ServiceOrderView{
        ServiceOrderItem{ServiceOrderItemId{"item-1"}, ContentRef::song("song-1"), 0},
        QStringLiteral("Song")}});
    ContentRef target = ContentRef::sermon("unset");
    AssetId asset{"unset"};
    bool callback_called = false;
    widget.set_asset_drop_handler([&](ContentRef content, AssetId dropped) {
        callback_called = true;
        target = std::move(content);
        asset = std::move(dropped);
    });

    auto* list = widget.findChild<QListWidget*>(QStringLiteral("service-order-list"));
    REQUIRE(list != nullptr);
    widget.show();
    QTest::qWait(20);
    auto* mime = shekinah::ui::make_asset_mime_data(AssetId{"asset-1"});
    const auto position = list->visualItemRect(list->item(0)).center();
    REQUIRE(list->acceptDrops());
    REQUIRE(list->visualItemRect(list->item(0)).contains(position));
    REQUIRE(shekinah::ui::asset_id_from_mime_data(mime).has_value());
    QDragEnterEvent enter{position, Qt::CopyAction, mime, Qt::LeftButton, Qt::NoModifier};
    QApplication::sendEvent(list->viewport(), &enter);
    REQUIRE(enter.isAccepted());
    QDragMoveEvent move{position, Qt::CopyAction, mime, Qt::LeftButton, Qt::NoModifier};
    QApplication::sendEvent(list->viewport(), &move);
    REQUIRE(move.isAccepted());
    QDropEvent event{position, Qt::CopyAction, mime, Qt::LeftButton, Qt::NoModifier};
    QApplication::sendEvent(list->viewport(), &event);

    REQUIRE(callback_called);
    REQUIRE(event.isAccepted());
    REQUIRE(target == ContentRef::song("song-1"));
    REQUIRE(asset == AssetId{"asset-1"});
    delete mime;
}

TEST_CASE("service order reports reusable content drops as presentation references") {
    ensure_qapplication();
    ServiceOrderWidget widget;
    widget.set_items({ServiceOrderView{
        ServiceOrderItem{ServiceOrderItemId{"item-1"}, ContentRef::song("song-1"), 0},
        QStringLiteral("Song")}});
    ContentRef dropped = ContentRef::sermon("unset");
    widget.set_content_drop_handler([&](ContentRef content) { dropped = std::move(content); });
    widget.show();
    QTest::qWait(20);

    auto* list = widget.findChild<QListWidget*>(QStringLiteral("service-order-list"));
    REQUIRE(list != nullptr);
    auto* mime = shekinah::ui::make_content_mime_data(ContentRef::song("song-2"));
    const auto position = list->visualItemRect(list->item(0)).center();
    QDragEnterEvent enter{position, Qt::CopyAction, mime, Qt::LeftButton, Qt::NoModifier};
    QApplication::sendEvent(list->viewport(), &enter);
    REQUIRE(enter.isAccepted());
    QDropEvent drop{position, Qt::CopyAction, mime, Qt::LeftButton, Qt::NoModifier};
    QApplication::sendEvent(list->viewport(), &drop);

    REQUIRE(drop.isAccepted());
    REQUIRE(dropped == ContentRef::song("song-2"));
    delete mime;
}

TEST_CASE("service order reports removal of the selected presentation reference") {
    ensure_qapplication();
    ServiceOrderWidget widget;
    widget.set_items({
        ServiceOrderView{ServiceOrderItem{ServiceOrderItemId{"item-1"}, ContentRef::song("song-1"), 0}, "Song 1"},
        ServiceOrderView{ServiceOrderItem{ServiceOrderItemId{"item-2"}, ContentRef::song("song-2"), 1}, "Song 2"},
    });
    ServiceOrderItemId removed;
    widget.set_remove_handler([&](ServiceOrderItemId id) { removed = std::move(id); });
    widget.show();
    widget.select_row(1);
    QTest::qWait(20);

    auto* remove = widget.findChild<QPushButton*>(QStringLiteral("service-order-remove"));
    REQUIRE(remove != nullptr);
    remove->click();

    REQUIRE(removed == ServiceOrderItemId{"item-2"});
}

TEST_CASE("service order ignores unsupported or out-of-row asset drops") {
    ensure_qapplication();
    ServiceOrderWidget widget;
    widget.set_items({ServiceOrderView{
        ServiceOrderItem{ServiceOrderItemId{"item-1"}, ContentRef::song("song-1"), 0},
        QStringLiteral("Song")}});
    bool callback_called = false;
    widget.set_asset_drop_handler([&](ContentRef, AssetId) { callback_called = true; });
    widget.show();
    QTest::qWait(20);

    auto* list = widget.findChild<QListWidget*>(QStringLiteral("service-order-list"));
    REQUIRE(list != nullptr);
    QMimeData unsupported;
    QDropEvent unsupported_event{list->visualItemRect(list->item(0)).center(), Qt::CopyAction,
                                 &unsupported, Qt::LeftButton, Qt::NoModifier};
    QApplication::sendEvent(list->viewport(), &unsupported_event);
    REQUIRE_FALSE(unsupported_event.isAccepted());

    auto* mime = shekinah::ui::make_asset_mime_data(AssetId{"asset-1"});
    const auto outside = list->viewport()->rect().bottomRight();
    QDropEvent outside_event{outside, Qt::CopyAction, mime, Qt::LeftButton, Qt::NoModifier};
    QApplication::sendEvent(list->viewport(), &outside_event);
    REQUIRE_FALSE(outside_event.isAccepted());
    REQUIRE_FALSE(callback_called);
    delete mime;
}
