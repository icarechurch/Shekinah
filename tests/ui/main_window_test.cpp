#include <catch2/catch_test_macros.hpp>

#include <QApplication>

#include "shekinah/ui/main_window.hpp"

using namespace shekinah::ui;

namespace {

QApplication& ensure_qapplication() {
    static int argc = 1;
    static char application_name[] = "shekinah_main_window_tests";
    static char* argv[] = {application_name, nullptr};
    static QApplication application{argc, argv};
    return application;
}

}  // namespace

TEST_CASE("main window composes the operator page and bottom Library dock") {
    ensure_qapplication();
    ShekinahWindow window;
    REQUIRE(window.windowTitle() == QStringLiteral("Shekinah"));
    REQUIRE(window.centralWidget()->objectName() == QStringLiteral("operator-page"));
    REQUIRE(window.findChild<QWidget*>(QStringLiteral("library-dock")) != nullptr);
}
