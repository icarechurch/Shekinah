#include <catch2/catch_test_macros.hpp>

#include <QApplication>
#include <QAction>

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
    REQUIRE(window.findChild<QAction*>(QStringLiteral("show-audience-output")) != nullptr);
}

TEST_CASE("main window applies the bounded live-media admission thresholds") {
    ensure_qapplication();
    ShekinahWindow window;

    window.sample_memory_usage(270'000'000);
    REQUIRE(window.last_working_set_bytes() == 270'000'000);
    REQUIRE_FALSE(window.live_media_admitted());

    window.sample_memory_usage(100'000'000);
    REQUIRE(window.live_media_admitted());
}
