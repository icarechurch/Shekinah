#include <catch2/catch_test_macros.hpp>

#include <QApplication>
#include <QPushButton>
#include <QTest>

#include "shekinah/ui/service_order_widget.hpp"

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
