#include <catch2/catch_test_macros.hpp>

#include <QApplication>
#include <QDialogButtonBox>
#include <QListWidget>
#include <QPushButton>

#include <fstream>

#include "shekinah/ui/sermon_editor_dialog.hpp"

namespace {

QApplication& ensure_qapplication() {
    static int argc = 1;
    static char application_name[] = "shekinah_sermon_editor_tests";
    static char* argv[] = {application_name, nullptr};
    static QApplication application{argc, argv};
    return application;
}

}  // namespace

TEST_CASE("Sermon editor saves a user-defined title and slide order") {
    ensure_qapplication();
    shekinah::ui::SermonEditorDialog dialog;
    dialog.set_title(QStringLiteral("Sunday Message"));
    dialog.set_files({"C:/slides/Slide 1.png", "C:/slides/Slide 2.png"});
    dialog.findChild<QListWidget*>(QStringLiteral("sermon-files"))->setCurrentRow(1);
    dialog.findChild<QPushButton*>(QStringLiteral("sermon-move-up"))->click();

    REQUIRE(dialog.title() == QStringLiteral("Sunday Message"));
    REQUIRE(dialog.file_count() == 2);
    const auto files = dialog.files();
    REQUIRE(files[0].filename().string() == "Slide 2.png");
    REQUIRE(files[1].filename().string() == "Slide 1.png");

    auto* buttons = dialog.findChild<QDialogButtonBox*>(QStringLiteral("sermon-buttons"));
    REQUIRE(buttons != nullptr);
    buttons->button(QDialogButtonBox::Save)->click();
    REQUIRE(dialog.result() == QDialog::Accepted);
}
