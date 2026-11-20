#include <catch2/catch_test_macros.hpp>

#include <QApplication>

#include "shekinah/ui/operator_page.hpp"
#include "shekinah/ui/program_status_widget.hpp"

using namespace shekinah::ui;

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
}
