#include <catch2/catch_test_macros.hpp>

#include <QApplication>
#include <QDialogButtonBox>
#include <QLabel>
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
    dialog.set_files({"C:/slides/Slide 1.jpg", "C:/slides/Clip 2.MP4"});
    dialog.findChild<QListWidget*>(QStringLiteral("sermon-files"))->setCurrentRow(1);
    dialog.findChild<QPushButton*>(QStringLiteral("sermon-move-up"))->click();

    REQUIRE(dialog.title() == QStringLiteral("Sunday Message"));
    REQUIRE(dialog.file_count() == 2);
    auto* description = dialog.findChild<QLabel*>(QStringLiteral("sermon-description"));
    REQUIRE(description != nullptr);
    REQUIRE(description->text().contains(QStringLiteral("media"), Qt::CaseInsensitive));

    auto* add_button = dialog.findChild<QPushButton*>(QStringLiteral("sermon-add-files"));
    REQUIRE(add_button != nullptr);
    REQUIRE(add_button->text() == QStringLiteral("Add Media"));

    const auto files = dialog.files();
    REQUIRE(files[0].filename().string() == "Clip 2.MP4");
    REQUIRE(files[1].filename().string() == "Slide 1.jpg");

    auto* buttons = dialog.findChild<QDialogButtonBox*>(QStringLiteral("sermon-buttons"));
    REQUIRE(buttons != nullptr);
    buttons->button(QDialogButtonBox::Save)->click();
    REQUIRE(dialog.result() == QDialog::Accepted);
}
