#include <catch2/catch_test_macros.hpp>

#include <QApplication>
#include <QAction>
#include <QKeySequence>
#include <QTest>

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
    REQUIRE(window.findChild<QWidget*>(QStringLiteral("presentation-section")) != nullptr);
}

TEST_CASE("main window exposes project and presentation actions") {
    ensure_qapplication();
    ShekinahWindow window;

    const auto require_action = [&](const QString& name) {
        auto* action = window.findChild<QAction*>(name);
        REQUIRE(action != nullptr);
        return action;
    };
    REQUIRE(require_action(QStringLiteral("new-project"))->shortcut() == QKeySequence::New);
    REQUIRE(require_action(QStringLiteral("open-project"))->shortcut() == QKeySequence::Open);
    REQUIRE(require_action(QStringLiteral("previous-cue"))->shortcut() == QKeySequence{Qt::Key_PageUp});
    REQUIRE(require_action(QStringLiteral("next-cue"))->shortcut() == QKeySequence{Qt::Key_PageDown});
    REQUIRE(require_action(QStringLiteral("clear-program"))->shortcut() == QKeySequence{Qt::Key_Escape});
    REQUIRE(require_action(QStringLiteral("blackout"))->shortcut() == QKeySequence{Qt::Key_B});
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

TEST_CASE("main window resizes smoothly to compact laptop and split-screen dimensions") {
    ensure_qapplication();
    ShekinahWindow window;
    window.show();
    QTest::qWait(30);

    for (const QSize target : {QSize{860, 560}, QSize{1024, 640}, QSize{1280, 720}}) {
        window.resize(target);
        QTest::qWait(30);
        REQUIRE(window.width() <= target.width() + 2);
        REQUIRE(window.height() <= target.height() + 2);
        for (const auto& child_name : {
                 QStringLiteral("service-order"),
                 QStringLiteral("cue-grid"),
                 QStringLiteral("program-status"),
                 QStringLiteral("library-dock"),
             }) {
            auto* pane = window.findChild<QWidget*>(child_name);
            REQUIRE(pane != nullptr);
            REQUIRE(pane->isVisible());
            REQUIRE(pane->width() > 0);
            REQUIRE(pane->height() > 0);
        }
    }
}
