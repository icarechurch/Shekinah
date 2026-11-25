#include <catch2/catch_test_macros.hpp>

#include <QAction>
#include <QApplication>
#include <QImage>
#include <QMenu>
#include <QSettings>
#include <QTest>

#include <cmath>
#include <set>

#include "shekinah/ui/cue_grid_widget.hpp"
#include "shekinah/ui/inspector_dialog.hpp"
#include "shekinah/ui/library_dock_widget.hpp"
#include "shekinah/ui/main_window.hpp"
#include "shekinah/ui/operator_page.hpp"
#include "shekinah/ui/program_status_widget.hpp"
#include "shekinah/ui/reflow_editor_dialog.hpp"
#include "shekinah/ui/reflow_text_edit.hpp"
#include "shekinah/ui/sermon_editor_dialog.hpp"
#include "shekinah/ui/service_order_widget.hpp"
#include "shekinah/ui/support/theme.hpp"
#include "test_project.hpp"

using namespace shekinah::ui;

namespace {

QApplication& ensure_qapplication() {
    static int argc = 1;
    static char application_name[] = "shekinah_theme_tests";
    static char* argv[] = {application_name, nullptr};
    static QApplication application{argc, argv};
    return application;
}

class ScopedThemeReset {
public:
    ScopedThemeReset() : previous_(ThemeManager::instance().current_theme_id()) {}
    ~ScopedThemeReset() { ThemeManager::instance().set_theme(previous_); }

private:
    ThemeId previous_;
};

double relative_luminance(const QColor& c) {
    const auto channel = [](double v) {
        return v <= 0.03928 ? v / 12.92 : std::pow((v + 0.055) / 1.055, 2.4);
    };
    return 0.2126 * channel(c.redF()) + 0.7152 * channel(c.greenF()) +
           0.0722 * channel(c.blueF());
}

double contrast_ratio(const QColor& a, const QColor& b) {
    const double l1 = relative_luminance(a);
    const double l2 = relative_luminance(b);
    const double hi = std::max(l1, l2);
    const double lo = std::min(l1, l2);
    return (hi + 0.05) / (lo + 0.05);
}

}  // namespace

TEST_CASE("theme manager exposes light, dark, and additional curated themes with strong contrast") {
    ensure_qapplication();
    ScopedThemeReset reset;

    REQUIRE(ThemeManager::instance().current_theme_id() == ThemeId::dark);

    const auto& themes = ThemeManager::available_themes();
    REQUIRE(themes.size() >= 6);

    std::set<QString> keys;
    bool has_light = false;
    bool has_dark = false;
    for (const auto& info : themes) {
        REQUIRE(!info.key.isEmpty());
        REQUIRE(!info.label.isEmpty());
        REQUIRE(keys.insert(info.key).second);
        if (info.id == ThemeId::light) {
            has_light = true;
            REQUIRE_FALSE(info.is_dark);
        }
        if (info.id == ThemeId::dark) {
            has_dark = true;
            REQUIRE(info.is_dark);
        }
        if (info.id == ThemeId::system) {
            continue;
        }
        const auto& p = ThemeManager::palette_for(info.id);
        REQUIRE(p.is_dark == info.is_dark);
        if (info.is_dark) {
            REQUIRE(p.window_bg.lightness() < 128);
        } else {
            REQUIRE(p.window_bg.lightness() >= 128);
        }
        REQUIRE(contrast_ratio(p.text_primary, p.window_bg) >= 7.0);
        REQUIRE(contrast_ratio(p.text_primary, p.panel_bg) >= 7.0);
        REQUIRE(contrast_ratio(p.text_selected, p.card_selected_bg) >= 4.5);
        REQUIRE(contrast_ratio(p.text_selected, p.list_selected_bg) >= 4.5);
        REQUIRE(contrast_ratio(p.text_on_accent, p.accent) >= 3.5);
        REQUIRE(contrast_ratio(p.reflow_break_fg, p.reflow_break_bg) >= 7.0);
    }
    REQUIRE(has_light);
    REQUIRE(has_dark);
}

TEST_CASE("theme manager switches themes by id and key and persists via QSettings") {
    ensure_qapplication();
    ScopedThemeReset reset;

    auto project = TestProject::create();
    const auto ini_path = QString::fromStdString((project.root() / "ui.ini").string());

    int notifications = 0;
    const auto conn = QObject::connect(
        &ThemeManager::instance(),
        &ThemeManager::theme_changed,
        [&](ThemeId) { ++notifications; });

    REQUIRE(ThemeManager::instance().set_theme_by_key(QStringLiteral("light")));
    REQUIRE(ThemeManager::instance().current_theme_id() == ThemeId::light);
    REQUIRE(ThemeManager::instance().resolved_theme_id() == ThemeId::light);
    REQUIRE_FALSE(ThemeManager::instance().palette().is_dark);
    REQUIRE(notifications == 1);

    REQUIRE_FALSE(ThemeManager::instance().set_theme_by_key(QStringLiteral("nonexistent-theme")));
    REQUIRE(ThemeManager::instance().current_theme_id() == ThemeId::light);

    {
        QSettings settings{ini_path, QSettings::IniFormat};
        ThemeManager::instance().save_to_settings(settings);
        settings.sync();
    }

    ThemeManager::instance().set_theme(ThemeId::midnight);
    REQUIRE(ThemeManager::instance().current_theme_id() == ThemeId::midnight);

    {
        QSettings settings{ini_path, QSettings::IniFormat};
        ThemeManager::instance().load_from_settings(settings);
    }
    REQUIRE(ThemeManager::instance().current_theme_id() == ThemeId::light);

    QObject::disconnect(conn);
}

TEST_CASE("main window View Theme menu switches themes live across workspace and dialogs") {
    ensure_qapplication();
    ScopedThemeReset reset;

    ShekinahWindow window;
    window.resize(1024, 680);
    window.show();
    QTest::qWait(20);

    auto* view_menu = window.findChild<QMenu*>(QStringLiteral("view-menu"));
    auto* theme_menu = window.findChild<QMenu*>(QStringLiteral("theme-menu"));
    REQUIRE(view_menu != nullptr);
    REQUIRE(theme_menu != nullptr);

    auto* dark_action = window.findChild<QAction*>(QStringLiteral("theme-dark"));
    auto* light_action = window.findChild<QAction*>(QStringLiteral("theme-light"));
    auto* midnight_action = window.findChild<QAction*>(QStringLiteral("theme-midnight"));
    auto* slate_action = window.findChild<QAction*>(QStringLiteral("theme-slate"));
    auto* warm_action = window.findChild<QAction*>(QStringLiteral("theme-warm-light"));
    auto* hc_action = window.findChild<QAction*>(QStringLiteral("theme-high-contrast"));
    auto* system_action = window.findChild<QAction*>(QStringLiteral("theme-system"));

    REQUIRE(dark_action != nullptr);
    REQUIRE(light_action != nullptr);
    REQUIRE(midnight_action != nullptr);
    REQUIRE(slate_action != nullptr);
    REQUIRE(warm_action != nullptr);
    REQUIRE(hc_action != nullptr);
    REQUIRE(system_action != nullptr);
    REQUIRE(dark_action->isChecked());

    InspectorDialog inspector{&window};
    ReflowEditorDialog reflow{&window};
    reflow.set_text("Line one\nLine two");
    reflow.editor()->set_cursor_utf8_offset(9);
    REQUIRE(reflow.editor()->insert_slide_break());
    SermonEditorDialog sermon{&window};

    light_action->trigger();
    QTest::qWait(20);
    REQUIRE(window.theme() == ThemeId::light);
    REQUIRE(light_action->isChecked());
    REQUIRE_FALSE(dark_action->isChecked());
    REQUIRE(window.palette().color(QPalette::Window) ==
            ThemeManager::palette_for(ThemeId::light).window_bg);
    REQUIRE(inspector.palette().color(QPalette::Window) ==
            ThemeManager::palette_for(ThemeId::light).panel_bg);
    REQUIRE(reflow.palette().color(QPalette::Window) ==
            ThemeManager::palette_for(ThemeId::light).panel_bg);
    REQUIRE(sermon.palette().color(QPalette::Window) ==
            ThemeManager::palette_for(ThemeId::light).panel_bg);

    for (auto* action : {midnight_action, slate_action, warm_action, hc_action, dark_action}) {
        action->trigger();
        QTest::qWait(15);
        REQUIRE(action->isChecked());
    }
    REQUIRE(window.theme() == ThemeId::dark);
}

TEST_CASE("widgets repaint with active theme colors when switched dynamically") {
    ensure_qapplication();
    ScopedThemeReset reset;

    OperatorPage page;
    page.resize(960, 600);
    page.cue_grid()->set_cues({shekinah::ui::CueView{
        shekinah::domain::CueId{"cue-1"},
        QStringLiteral("Verse"),
        QStringLiteral("Amazing grace"),
        1,
    }});
    page.show();
    QTest::qWait(20);

    auto render_center_pixel = [&page]() {
        QImage img{page.size(), QImage::Format_ARGB32_Premultiplied};
        img.fill(Qt::transparent);
        page.render(&img);
        const QPoint sample = page.cue_grid()->mapTo(
            &page, QPoint{page.cue_grid()->width() - 12, page.cue_grid()->height() - 12});
        return img.pixelColor(sample);
    };

    ThemeManager::instance().set_theme(ThemeId::dark);
    QTest::qWait(20);
    const QColor dark_pixel = render_center_pixel();

    ThemeManager::instance().set_theme(ThemeId::light);
    QTest::qWait(20);
    const QColor light_pixel = render_center_pixel();

    ThemeManager::instance().set_theme(ThemeId::warm_light);
    QTest::qWait(20);
    const QColor warm_pixel = render_center_pixel();

    REQUIRE(dark_pixel != light_pixel);
    REQUIRE(light_pixel != warm_pixel);
    REQUIRE(dark_pixel == ThemeManager::palette_for(ThemeId::dark).window_bg);
    REQUIRE(light_pixel == ThemeManager::palette_for(ThemeId::light).window_bg);
    REQUIRE(warm_pixel == ThemeManager::palette_for(ThemeId::warm_light).window_bg);
}
