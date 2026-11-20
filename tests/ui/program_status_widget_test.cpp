#include <catch2/catch_test_macros.hpp>

#include <QApplication>
#include <QLabel>
#include <QPushButton>
#include <QTest>

#include "shekinah/ui/program_status_widget.hpp"

using namespace shekinah::domain;
using namespace shekinah::ui;

namespace {

QApplication& ensure_qapplication() {
    static int argc = 1;
    static char application_name[] = "shekinah_program_status_tests";
    static char* argv[] = {application_name, nullptr};
    static QApplication application{argc, argv};
    return application;
}

}  // namespace

TEST_CASE("Program Status dispatches all four global controls") {
    ensure_qapplication();
    ProgramStatusWidget widget;
    int previous = 0;
    int next = 0;
    int clear = 0;
    int black = 0;
    widget.set_previous_handler([&] { ++previous; });
    widget.set_next_handler([&] { ++next; });
    widget.set_clear_handler([&] { ++clear; });
    widget.set_black_handler([&] { ++black; });

    widget.findChild<QPushButton*>(QStringLiteral("previous"))->click();
    widget.findChild<QPushButton*>(QStringLiteral("next"))->click();
    widget.findChild<QPushButton*>(QStringLiteral("clear"))->click();
    widget.findChild<QPushButton*>(QStringLiteral("black"))->click();

    REQUIRE(previous == 1);
    REQUIRE(next == 1);
    REQUIRE(clear == 1);
    REQUIRE(black == 1);
}

TEST_CASE("Program Status renders the live cue label") {
    ensure_qapplication();
    ProgramStatusWidget widget;
    widget.set_program(ProgramState{
        ProgramMode::cue,
        CueRef{ContentRef::song("song-a"), CueId{"bridge-1"}},
    });
    REQUIRE(widget.findChild<QLabel*>(QStringLiteral("program-mode"))->text() == QStringLiteral("LIVE"));
    REQUIRE(widget.findChild<QLabel*>(QStringLiteral("program-cue"))->text() == QStringLiteral("bridge-1"));
}
