#include <catch2/catch_test_macros.hpp>

#include <QAction>
#include <QApplication>
#include <QImage>
#include <QTest>

#include "test_project.hpp"
#include "shekinah/media/audience_output.hpp"
#include "shekinah/ui/cue_grid_widget.hpp"
#include "shekinah/ui/library_dock_widget.hpp"
#include "shekinah/ui/main_window.hpp"
#include "shekinah/ui/operator_page.hpp"

namespace {

QApplication& ensure_qapplication() {
    static int argc = 1;
    static char application_name[] = "shekinah_e2e_tests";
    static char* argv[] = {application_name, nullptr};
    static QApplication application{argc, argv};
    return application;
}

}  // namespace

TEST_CASE("operator cue click reaches the Audience Output boundary") {
    ensure_qapplication();
    auto project = TestProject::create();
    const auto source = project.root() / "Slide 1.png";
    QImage image{640, 360, QImage::Format_RGB32};
    image.fill(Qt::yellow);
    REQUIRE(image.save(QString::fromStdString(source.string()), "PNG"));

    shekinah::ui::ShekinahWindow window;
    REQUIRE(window.open_project(project.root() / "Sunday"));
    const auto sermon = window.import_sermon_files({source}, "E2E Sermon");
    REQUIRE(sermon);
    window.library_dock()->add_sermon(*sermon);
    window.library_dock()->show_sermons();
    window.library_dock()->activate_sermon_row(0);
    auto* output = window.audience_output();
    window.show();
    QTest::qWait(20);

    auto* cue = window.operator_page()->cue_grid()->findChild<QWidget*>(QStringLiteral("cue-slide-0"));
    REQUIRE(cue != nullptr);
    REQUIRE_FALSE(cue->property("cue-background-path").toString().isEmpty());
    QTest::mouseClick(cue, Qt::LeftButton, Qt::NoModifier, cue->rect().center());

    REQUIRE(output->program().mode == shekinah::domain::ProgramMode::cue);
    REQUIRE(output->slide_image_loaded());
    REQUIRE(cue->property("cue-live").toBool());

    auto* clear = window.findChild<QAction*>(QStringLiteral("clear-program"));
    REQUIRE(clear != nullptr);
    clear->trigger();
    REQUIRE_FALSE(cue->property("cue-live").toBool());
}
