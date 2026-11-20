#include <catch2/catch_test_macros.hpp>

#include <QApplication>
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
