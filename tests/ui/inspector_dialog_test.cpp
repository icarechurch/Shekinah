#include <catch2/catch_test_macros.hpp>

#include <QApplication>
#include <QMenu>

#include "shekinah/ui/inspector_dialog.hpp"
#include "shekinah/ui/operator_page.hpp"
#include "shekinah/ui/program_status_widget.hpp"

using namespace shekinah::domain;
using namespace shekinah::ui;

namespace {

QApplication& ensure_qapplication() {
    static int argc = 1;
    static char application_name[] = "shekinah_inspector_tests";
    static char* argv[] = {application_name, nullptr};
    static QApplication application{argc, argv};
    return application;
}

}  // namespace

TEST_CASE("right-click context menu includes Inspector and per-slide actions") {
    ensure_qapplication();
    OperatorPage page;
    auto* menu = page.create_context_menu(CueId{"bridge-1"});
    REQUIRE(menu != nullptr);
    REQUIRE(menu->actions().size() == 5);
    REQUIRE(menu->actions()[0]->text() == QStringLiteral("Open Inspector"));
    REQUIRE(menu->actions()[1]->text() == QStringLiteral("Edit slide"));
    REQUIRE(menu->actions()[2]->text() == QStringLiteral("Set background"));
    REQUIRE(menu->actions()[3]->text() == QStringLiteral("Duplicate"));
    REQUIRE(menu->actions()[4]->text() == QStringLiteral("Delete"));
    menu->deleteLater();
}

TEST_CASE("Set background dispatches the selected cue to the operator boundary") {
    ensure_qapplication();
    OperatorPage page;
    CueId requested;
    page.set_background_handler([&requested](CueId cue) { requested = std::move(cue); });
    auto* menu = page.create_context_menu(CueId{"bridge-1"});

    menu->actions()[2]->trigger();

    REQUIRE(requested == CueId{"bridge-1"});
    menu->deleteLater();
}

TEST_CASE("Inspector is a non-live floating popup") {
    ensure_qapplication();
    InspectorDialog dialog;
    dialog.show_inspector(ContentRef::song("song-a"), CueId{"bridge-1"});
    REQUIRE(dialog.isVisible());
    REQUIRE(dialog.windowModality() == Qt::NonModal);
    REQUIRE((dialog.windowFlags() & Qt::Tool) != 0);
    REQUIRE(dialog.requested_cue().value == "bridge-1");
    dialog.close();
    REQUIRE_FALSE(dialog.isVisible());
}
