#include <catch2/catch_test_macros.hpp>

#include <QApplication>
#include <QSplitter>
#include <QTest>

#include "shekinah/ui/cue_grid_widget.hpp"
#include "shekinah/ui/operator_page.hpp"
#include "shekinah/ui/program_status_widget.hpp"
#include "shekinah/ui/service_order_widget.hpp"

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
    REQUIRE(workspace->count() == 2);
    REQUIRE(page.service_order_widget()->isVisible());
    REQUIRE(page.cue_grid()->isVisible());
    REQUIRE(page.cue_grid()->width() > 0);
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
