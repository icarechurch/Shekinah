#include <catch2/catch_test_macros.hpp>

#include <QApplication>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QProgressBar>
#include <QPushButton>
#include <QScrollArea>
#include <QStackedWidget>
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

TEST_CASE("Program Status preview carries the actual slide content") {
    ensure_qapplication();
    ProgramStatusWidget widget;
    widget.set_slide_preview(QStringLiteral("First lyric line\nSecond lyric line"));

    auto* preview = widget.findChild<QWidget*>(QStringLiteral("program-preview"));
    REQUIRE(preview != nullptr);
    REQUIRE(preview->property("slide-text").toString() ==
            QStringLiteral("First lyric line\nSecond lyric line"));
}

TEST_CASE("Program Status exposes the right-side output panel") {
    ensure_qapplication();
    ProgramStatusWidget widget;

    REQUIRE(widget.findChild<QLabel*>(QStringLiteral("program-status-title"))->text() ==
            QStringLiteral("PROGRAM STATUS"));
    REQUIRE(widget.findChild<QWidget*>(QStringLiteral("program-preview")) != nullptr);
    REQUIRE(widget.findChild<QLabel*>(QStringLiteral("output-target"))->text() ==
            QStringLiteral("Output: Monitor 2"));
    REQUIRE(widget.findChild<QLabel*>(QStringLiteral("audience-mode"))->text() ==
            QStringLiteral("Audience full-screen"));
    REQUIRE(widget.findChild<QWidget*>(QStringLiteral("global-controls")) != nullptr);
}

TEST_CASE("Program Status quick-action rail and clear-all button dispatch handlers") {
    ensure_qapplication();
    ProgramStatusWidget widget;
    int clear_calls = 0;
    int black_calls = 0;
    int bg_calls = 0;
    int inspect_calls = 0;
    int movable_calls = 0;

    widget.set_clear_handler([&] { ++clear_calls; });
    widget.set_black_handler([&] { ++black_calls; });
    widget.set_choose_background_handler([&] { ++bg_calls; });
    widget.set_inspect_live_handler([&] { ++inspect_calls; });
    widget.set_show_movable_handler([&] { ++movable_calls; });

    widget.findChild<QPushButton*>(QStringLiteral("quick-clear-all"))->click();
    widget.findChild<QPushButton*>(QStringLiteral("rail-clear-cue"))->click();
    widget.findChild<QPushButton*>(QStringLiteral("rail-blackout"))->click();
    widget.findChild<QPushButton*>(QStringLiteral("rail-background"))->click();
    widget.findChild<QPushButton*>(QStringLiteral("rail-inspector"))->click();
    widget.findChild<QPushButton*>(QStringLiteral("rail-audience"))->click();

    REQUIRE(clear_calls == 2);
    REQUIRE(black_calls == 1);
    REQUIRE(bg_calls == 1);
    REQUIRE(inspect_calls == 1);
    REQUIRE(movable_calls == 1);
}

TEST_CASE("Program Status segmented tabs switch drawer views and keep controls below status") {
    ensure_qapplication();
    ProgramStatusWidget widget;
    widget.resize(320, 700);
    widget.show();
    QTest::qWait(20);

    auto* stack = widget.findChild<QStackedWidget*>(QStringLiteral("drawer-stack"));
    REQUIRE(stack != nullptr);
    REQUIRE(stack->currentIndex() == 0);

    widget.findChild<QPushButton*>(QStringLiteral("tab-screens"))->click();
    REQUIRE(stack->currentIndex() == 1);

    widget.findChild<QPushButton*>(QStringLiteral("tab-cues"))->click();
    REQUIRE(stack->currentIndex() == 2);

    widget.findChild<QPushButton*>(QStringLiteral("tab-live"))->click();
    REQUIRE(stack->currentIndex() == 0);
    REQUIRE(widget.global_controls_below_status());
}

TEST_CASE("Program Status displays live slide metadata, next cue, and memory budget") {
    ensure_qapplication();
    ProgramStatusWidget widget;
    widget.set_program(ProgramState{
        ProgramMode::cue,
        CueRef{ContentRef::song("song-a"), CueId{"song-a-slide-1"}},
    });
    widget.set_live_details(LiveSlideDetails{
        QStringLiteral("Goodness of God"),
        QStringLiteral("Chorus"),
        QStringLiteral("All my life You have been faithful"),
        QStringLiteral("worship-bg.png"),
        QStringLiteral("Bridge"),
        QStringLiteral("Your goodness is running after"),
        2,
        4,
        true,
        false,
        FitMode::cover,
    });
    widget.set_memory_status(150ULL * 1024ULL * 1024ULL, true);

    REQUIRE(widget.findChild<QLabel*>(QStringLiteral("live-section-badge"))->text() ==
            QStringLiteral("CHORUS"));
    REQUIRE(widget.findChild<QLabel*>(QStringLiteral("live-item-title"))->text() ==
            QStringLiteral("Goodness of God"));
    REQUIRE(widget.findChild<QLabel*>(QStringLiteral("live-slide-counter"))->text() ==
            QStringLiteral("2 / 4"));
    REQUIRE(widget.findChild<QLabel*>(QStringLiteral("live-lyrics-preview"))->text() ==
            QStringLiteral("All my life You have been faithful"));
    REQUIRE(widget.findChild<QLabel*>(QStringLiteral("next-section-badge"))->text() ==
            QStringLiteral("BRIDGE"));
    REQUIRE(widget.findChild<QLabel*>(QStringLiteral("next-summary-label"))->text() ==
            QStringLiteral("Your goodness is running after"));
    REQUIRE(widget.findChild<QProgressBar*>(QStringLiteral("slide-progress"))->value() == 50);
    REQUIRE(widget.findChild<QProgressBar*>(QStringLiteral("memory-usage-bar"))->value() == 150);
}

TEST_CASE("Program Status quick cues drawer filters and triggers live cue selection") {
    ensure_qapplication();
    ProgramStatusWidget widget;
    CueId activated;
    widget.set_take_cue_handler([&](CueId cue) { activated = std::move(cue); });
    widget.set_item_cues(QStringLiteral("Goodness"),
                         {
                             QuickCueItem{CueId{"c1"}, 1, QStringLiteral("Verse"), QStringLiteral("I love You Lord")},
                             QuickCueItem{CueId{"c2"}, 2, QStringLiteral("Chorus"), QStringLiteral("All my life")},
                         });

    auto* list = widget.findChild<QListWidget*>(QStringLiteral("quick-cues-list"));
    auto* filter = widget.findChild<QLineEdit*>(QStringLiteral("quick-cues-filter"));
    REQUIRE(list != nullptr);
    REQUIRE(filter != nullptr);
    REQUIRE(list->count() == 2);

    filter->setText(QStringLiteral("Chorus"));
    REQUIRE(list->item(0)->isHidden());
    REQUIRE_FALSE(list->item(1)->isHidden());

    widget.findChild<QPushButton*>(QStringLiteral("tab-cues"))->click();
    widget.show();
    QTest::qWait(20);
    const auto item_rect = list->visualItemRect(list->item(1));
    QTest::mouseClick(list->viewport(), Qt::LeftButton, Qt::NoModifier, item_rect.center());
    REQUIRE(activated == CueId{"c2"});
}

TEST_CASE("Program Status adapts to compact dimensions without locking height or clipping controls") {
    ensure_qapplication();
    ProgramStatusWidget widget;
    widget.set_program(ProgramState{
        ProgramMode::cue,
        CueRef{ContentRef::song("song-a"), CueId{"very-long-cue-identifier-slide-1"}},
    });
    widget.set_live_details(LiveSlideDetails{
        QStringLiteral("Very Long Worship Song Title For Narrow Sidebar Verification"),
        QStringLiteral("Chorus"),
        QStringLiteral("Line one of lyrics\nLine two of lyrics\nLine three of lyrics\nLine four of lyrics"),
        QStringLiteral("very_long_worship_background_loop_4k.mp4"),
        QStringLiteral("Bridge"),
        QStringLiteral("Next slide summary text that wraps cleanly inside the narrow drawer"),
        2,
        12,
        true,
        true,
        FitMode::cover,
    });
    widget.resize(270, 440);
    widget.show();
    QTest::qWait(30);

    REQUIRE(widget.width() <= 270);
    REQUIRE(widget.height() <= 440);
    REQUIRE(widget.global_controls_below_status());
    REQUIRE(widget.findChild<QScrollArea*>(QStringLiteral("drawer-page-live-scroll")) != nullptr);

    auto* controls = widget.findChild<QWidget*>(QStringLiteral("global-controls"));
    REQUIRE(controls != nullptr);
    for (const auto& name : {
             QStringLiteral("previous"),
             QStringLiteral("next"),
             QStringLiteral("clear"),
             QStringLiteral("black"),
         }) {
        auto* btn = controls->findChild<QPushButton*>(name);
        REQUIRE(btn != nullptr);
        REQUIRE(btn->x() >= 0);
        REQUIRE(btn->x() + btn->width() <= controls->width());
    }
}
