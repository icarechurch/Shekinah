#include <catch2/catch_test_macros.hpp>

#include <QApplication>
#include <QDialogButtonBox>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QImage>
#include <QLabel>
#include <QListWidget>
#include <QLineEdit>
#include <QMenu>
#include <QPushButton>
#include <QPlainTextEdit>
#include <QTest>

#include <algorithm>
#include <fstream>

#include "qt_drag_event_position.hpp"
#include "test_project.hpp"
#include "shekinah/media/audience_output.hpp"
#include "shekinah/ui/library_dock_widget.hpp"
#include "shekinah/ui/main_window.hpp"
#include "shekinah/ui/operator_page.hpp"
#include "shekinah/ui/program_status_widget.hpp"
#include "shekinah/ui/reflow_editor_dialog.hpp"
#include "shekinah/ui/reflow_text_edit.hpp"
#include "shekinah/ui/sermon_editor_dialog.hpp"
#include "shekinah/ui/service_order_widget.hpp"
#include "shekinah/ui/cue_grid_widget.hpp"
#include "shekinah/ui/support/asset_drag.hpp"
#include "shekinah/ui/support/content_drag.hpp"
#include "shekinah/storage/content_repository.hpp"
#include "shekinah/storage/project_store.hpp"

namespace {

QApplication& ensure_qapplication() {
    static int argc = 1;
    static char application_name[] = "shekinah_project_session_tests";
    static char* argv[] = {application_name, nullptr};
    static QApplication application{argc, argv};
    return application;
}

}  // namespace

TEST_CASE("asset dropped on a Song row sets its default background") {
    ensure_qapplication();
    auto project = TestProject::create();
    const auto image_source = project.root() / "background.png";
    QImage image{800, 450, QImage::Format_RGB32};
    image.fill(Qt::green);
    REQUIRE(image.save(QString::fromStdString(image_source.string()), "PNG"));
    const auto song_source = project.root() / "goodness.txt";
    std::ofstream{song_source} << "# Goodness\n[Verse]\nYou are good\n";

    shekinah::ui::ShekinahWindow window;
    REQUIRE(window.open_project(project.root() / "Sunday"));
    const auto asset = window.import_asset_file(image_source);
    const auto song = window.import_song_file(song_source);
    REQUIRE(asset);
    REQUIRE(song);
    window.library_dock()->add_song(*song);
    window.library_dock()->show_songs();
    window.library_dock()->activate_song_row(0);
    window.show();
    QTest::qWait(20);

    auto* list = window.operator_page()->service_order_widget()->findChild<QListWidget*>(
        QStringLiteral("service-order-list"));
    REQUIRE(list != nullptr);
    auto* mime = shekinah::ui::make_asset_mime_data(asset->id);
    const auto position = test_support::drag_event_position(
        list->visualItemRect(list->item(0)).center());
    QDragEnterEvent enter{position, Qt::CopyAction, mime, Qt::LeftButton, Qt::NoModifier};
    QApplication::sendEvent(list->viewport(), &enter);
    QDragMoveEvent move{position, Qt::CopyAction, mime, Qt::LeftButton, Qt::NoModifier};
    QApplication::sendEvent(list->viewport(), &move);
    QDropEvent drop{position, Qt::CopyAction, mime, Qt::LeftButton, Qt::NoModifier};
    QApplication::sendEvent(list->viewport(), &drop);
    delete mime;

    REQUIRE(drop.isAccepted());
    QTest::qWait(20);
    auto store = shekinah::storage::ProjectStore::create_or_open(project.root() / "Sunday");
    REQUIRE(store);
    shekinah::storage::SqliteContentRepository repository{*store};
    const auto saved_song = repository.load_song(song->id);
    REQUIRE(saved_song);
    REQUIRE(saved_song->default_background->asset == asset->id);
    auto* cue = window.operator_page()->cue_grid()->findChild<QWidget*>(
        QStringLiteral("cue-%1").arg(QString::fromStdString(song->slides[0].id.value)));
    REQUIRE(cue != nullptr);
    REQUIRE(cue->property("cue-background-path").toString() ==
            QString::fromStdString((project.root() / "Sunday" / asset->relative_path).string()));
}

TEST_CASE("Song slide context menus name their removable asset") {
    ensure_qapplication();
    auto project = TestProject::create();
    const auto image_source = project.root() / "background.png";
    QImage image{800, 450, QImage::Format_RGB32};
    image.fill(Qt::green);
    REQUIRE(image.save(QString::fromStdString(image_source.string()), "PNG"));
    const auto song_source = project.root() / "goodness.txt";
    std::ofstream{song_source} << "# Goodness\n[Verse]\nYou are good\n";

    shekinah::ui::ShekinahWindow window;
    REQUIRE(window.open_project(project.root() / "Sunday"));
    const auto asset = window.import_asset_file(image_source);
    const auto song = window.import_song_file(song_source);
    REQUIRE(asset);
    REQUIRE(song);
    window.library_dock()->add_song(*song);
    window.library_dock()->show_songs();
    window.library_dock()->activate_song_row(0);
    REQUIRE(window.set_slide_background(song->slides[0].id, asset->id));

    auto* menu = window.operator_page()->create_context_menu(song->slides[0].id);
    REQUIRE(menu != nullptr);
    const auto actions = menu->actions();
    const auto asset_name = QString::fromStdString(
        std::filesystem::path{asset->relative_path}.filename().string());
    REQUIRE(std::any_of(actions.begin(), actions.end(), [](const auto* action) {
        return action->text().startsWith(QStringLiteral("Remove asset "));
    }));
    REQUIRE(std::any_of(actions.begin(), actions.end(), [&](const auto* action) {
        return action->text() == QStringLiteral("Remove asset ") + asset_name;
    }));
    menu->deleteLater();
}

TEST_CASE("removing a Song asset clears its range and refreshes live output") {
    ensure_qapplication();
    auto project = TestProject::create();
    const auto first_source = project.root() / "first.png";
    const auto second_source = project.root() / "second.png";
    QImage first_image{800, 450, QImage::Format_RGB32};
    first_image.fill(Qt::green);
    QImage second_image{800, 450, QImage::Format_RGB32};
    second_image.fill(Qt::blue);
    REQUIRE(first_image.save(QString::fromStdString(first_source.string()), "PNG"));
    REQUIRE(second_image.save(QString::fromStdString(second_source.string()), "PNG"));
    const auto song_source = project.root() / "goodness.txt";
    std::ofstream{song_source}
        << "# Goodness\n[Verse]\nOne\n\n[Chorus]\nTwo\n\n[Bridge]\nThree\n";

    shekinah::ui::ShekinahWindow window;
    REQUIRE(window.open_project(project.root() / "Sunday"));
    const auto first_asset = window.import_asset_file(first_source);
    const auto second_asset = window.import_asset_file(second_source);
    const auto song = window.import_song_file(song_source);
    REQUIRE(first_asset);
    REQUIRE(second_asset);
    REQUIRE(song);
    window.library_dock()->add_song(*song);
    window.library_dock()->show_songs();
    window.library_dock()->activate_song_row(0);
    window.show();
    QTest::qWait(20);
    auto* output = window.audience_output();

    const auto drop_asset = [&](QWidget* target, const shekinah::domain::AssetId& asset_id) {
        auto* mime = shekinah::ui::make_asset_mime_data(asset_id);
        const auto position = test_support::drag_event_position(target->rect().center());
        QDragEnterEvent enter{position, Qt::CopyAction, mime, Qt::LeftButton, Qt::NoModifier};
        QApplication::sendEvent(target, &enter);
        QDragMoveEvent move{position, Qt::CopyAction, mime, Qt::LeftButton, Qt::NoModifier};
        QApplication::sendEvent(target, &move);
        QDropEvent drop{position, Qt::CopyAction, mime, Qt::LeftButton, Qt::NoModifier};
        QApplication::sendEvent(target, &drop);
        delete mime;
        REQUIRE(drop.isAccepted());
    };

    auto* first = window.operator_page()->cue_grid()->findChild<QWidget*>(
        QStringLiteral("cue-%1").arg(QString::fromStdString(song->slides[0].id.value)));
    auto* second = window.operator_page()->cue_grid()->findChild<QWidget*>(
        QStringLiteral("cue-%1").arg(QString::fromStdString(song->slides[1].id.value)));
    auto* third = window.operator_page()->cue_grid()->findChild<QWidget*>(
        QStringLiteral("cue-%1").arg(QString::fromStdString(song->slides[2].id.value)));
    REQUIRE(first != nullptr);
    REQUIRE(second != nullptr);
    REQUIRE(third != nullptr);
    QTest::mouseClick(third, Qt::LeftButton, Qt::NoModifier, third->rect().center());
    REQUIRE(output->program().live_cue.has_value());
    REQUIRE(output->program().live_cue->cue == song->slides[2].id);
    drop_asset(first, first_asset->id);
    QTest::qWait(20);
    second = window.operator_page()->cue_grid()->findChild<QWidget*>(
        QStringLiteral("cue-%1").arg(QString::fromStdString(song->slides[1].id.value)));
    REQUIRE(second != nullptr);
    drop_asset(second, second_asset->id);
    QTest::qWait(20);

    third = window.operator_page()->cue_grid()->findChild<QWidget*>(
        QStringLiteral("cue-%1").arg(QString::fromStdString(song->slides[2].id.value)));
    REQUIRE(third != nullptr);
    REQUIRE(third->isVisible());
    REQUIRE(third->property("cue-background-path").toString().isEmpty());
    QTest::qWait(100);
    REQUIRE(output->slide_image_loaded());

    auto* menu = window.operator_page()->create_context_menu(song->slides[2].id);
    REQUIRE(menu != nullptr);
    const auto actions = menu->actions();
    const auto remove_asset_it = std::find_if(actions.begin(), actions.end(), [](const auto* action) {
        return action->text().startsWith(QStringLiteral("Remove asset "));
    });
    REQUIRE(remove_asset_it != actions.end());
    auto* remove_asset = *remove_asset_it;
    REQUIRE(remove_asset != nullptr);
    remove_asset->trigger();
    menu->deleteLater();
    QTest::qWait(30);

    auto store = shekinah::storage::ProjectStore::create_or_open(project.root() / "Sunday");
    REQUIRE(store);
    shekinah::storage::SqliteContentRepository repository{*store};
    const auto saved_song = repository.load_song(song->id);
    REQUIRE(saved_song);
    REQUIRE(saved_song->slides[0].background->asset == first_asset->id);
    REQUIRE_FALSE(saved_song->slides[1].background.has_value());
    REQUIRE_FALSE(saved_song->slides[2].background.has_value());
    REQUIRE(repository.load_asset(second_asset->id));
    REQUIRE_FALSE(output->slide_image_loaded());
}

TEST_CASE("asset dropped on a non-selected Song row updates that Song") {
    ensure_qapplication();
    auto project = TestProject::create();
    const auto image_source = project.root() / "background.png";
    QImage image{800, 450, QImage::Format_RGB32};
    image.fill(Qt::yellow);
    REQUIRE(image.save(QString::fromStdString(image_source.string()), "PNG"));
    const auto song_source = project.root() / "goodness.txt";
    std::ofstream{song_source} << "# Goodness\n[Verse]\nYou are good\n";

    shekinah::ui::ShekinahWindow window;
    REQUIRE(window.open_project(project.root() / "Sunday"));
    const auto asset = window.import_asset_file(image_source);
    const auto first = window.import_song_file(song_source);
    const auto second = window.import_song_file(song_source);
    REQUIRE(asset);
    REQUIRE(first);
    REQUIRE(second);
    window.library_dock()->add_song(*first);
    window.library_dock()->add_song(*second);
    window.library_dock()->show_songs();
    window.library_dock()->activate_song_row(0);
    window.library_dock()->activate_song_row(1);
    window.show();
    QTest::qWait(20);

    auto* list = window.operator_page()->service_order_widget()->findChild<QListWidget*>(
        QStringLiteral("service-order-list"));
    REQUIRE(list != nullptr);
    REQUIRE(list->count() == 2);
    auto* mime = shekinah::ui::make_asset_mime_data(asset->id);
    const auto position = test_support::drag_event_position(
        list->visualItemRect(list->item(1)).center());
    QDragEnterEvent enter{position, Qt::CopyAction, mime, Qt::LeftButton, Qt::NoModifier};
    QApplication::sendEvent(list->viewport(), &enter);
    QDragMoveEvent move{position, Qt::CopyAction, mime, Qt::LeftButton, Qt::NoModifier};
    QApplication::sendEvent(list->viewport(), &move);
    QDropEvent drop{position, Qt::CopyAction, mime, Qt::LeftButton, Qt::NoModifier};
    QApplication::sendEvent(list->viewport(), &drop);
    delete mime;

    REQUIRE(drop.isAccepted());
    QTest::qWait(20);
    auto store = shekinah::storage::ProjectStore::create_or_open(project.root() / "Sunday");
    REQUIRE(store);
    shekinah::storage::SqliteContentRepository repository{*store};
    const auto saved_first = repository.load_song(first->id);
    const auto saved_second = repository.load_song(second->id);
    REQUIRE(saved_first);
    REQUIRE(saved_second);
    REQUIRE_FALSE(saved_first->default_background.has_value());
    REQUIRE(saved_second->default_background->asset == asset->id);
}

TEST_CASE("asset dropped on a Song slide applies through the following slides") {
    ensure_qapplication();
    auto project = TestProject::create();
    const auto image_source = project.root() / "background.png";
    QImage image{800, 450, QImage::Format_RGB32};
    image.fill(Qt::cyan);
    REQUIRE(image.save(QString::fromStdString(image_source.string()), "PNG"));
    const auto song_source = project.root() / "goodness.txt";
    std::ofstream{song_source}
        << "# Goodness\n[Verse]\nOne\n\n[Chorus]\nTwo\n\n[Bridge]\nThree\n";

    shekinah::ui::ShekinahWindow window;
    REQUIRE(window.open_project(project.root() / "Sunday"));
    const auto asset = window.import_asset_file(image_source);
    const auto song = window.import_song_file(song_source);
    REQUIRE(asset);
    REQUIRE(song);
    window.library_dock()->add_song(*song);
    window.library_dock()->show_songs();
    window.library_dock()->activate_song_row(0);
    window.show();
    QTest::qWait(20);

    auto* target = window.operator_page()->cue_grid()->findChild<QWidget*>(
        QStringLiteral("cue-%1").arg(QString::fromStdString(song->slides[1].id.value)));
    REQUIRE(target != nullptr);
    auto* mime = shekinah::ui::make_asset_mime_data(asset->id);
    const auto position = test_support::drag_event_position(target->rect().center());
    QDragEnterEvent enter{position, Qt::CopyAction, mime, Qt::LeftButton, Qt::NoModifier};
    QApplication::sendEvent(target, &enter);
    QDragMoveEvent move{position, Qt::CopyAction, mime, Qt::LeftButton, Qt::NoModifier};
    QApplication::sendEvent(target, &move);
    QDropEvent drop{position, Qt::CopyAction, mime, Qt::LeftButton, Qt::NoModifier};
    QApplication::sendEvent(target, &drop);
    delete mime;

    REQUIRE(drop.isAccepted());
    QTest::qWait(20);
    const auto background_path =
        QString::fromStdString((project.root() / "Sunday" / asset->relative_path).string());
    auto* first = window.operator_page()->cue_grid()->findChild<QWidget*>(
        QStringLiteral("cue-%1").arg(QString::fromStdString(song->slides[0].id.value)));
    auto* second = window.operator_page()->cue_grid()->findChild<QWidget*>(
        QStringLiteral("cue-%1").arg(QString::fromStdString(song->slides[1].id.value)));
    auto* third = window.operator_page()->cue_grid()->findChild<QWidget*>(
        QStringLiteral("cue-%1").arg(QString::fromStdString(song->slides[2].id.value)));
    REQUIRE(first != nullptr);
    REQUIRE(second != nullptr);
    REQUIRE(third != nullptr);
    REQUIRE(first->property("cue-background-path").toString().isEmpty());
    REQUIRE(second->property("cue-background-path").toString() == background_path);
    REQUIRE(third->property("cue-background-path").toString().isEmpty());

    auto store = shekinah::storage::ProjectStore::create_or_open(project.root() / "Sunday");
    REQUIRE(store);
    shekinah::storage::SqliteContentRepository repository{*store};
    const auto saved_song = repository.load_song(song->id);
    REQUIRE(saved_song);
    REQUIRE_FALSE(saved_song->slides[0].background.has_value());
    REQUIRE(saved_song->slides[1].background->asset == asset->id);
    REQUIRE(saved_song->slides[2].background->asset == asset->id);
}

TEST_CASE("asset slide drops refresh the live preview") {
    ensure_qapplication();
    auto project = TestProject::create();
    const auto image_source = project.root() / "background.png";
    QImage image{800, 450, QImage::Format_RGB32};
    image.fill(Qt::magenta);
    REQUIRE(image.save(QString::fromStdString(image_source.string()), "PNG"));
    const auto song_source = project.root() / "goodness.txt";
    std::ofstream{song_source}
        << "# Goodness\n[Verse]\nOne\n\n[Chorus]\nTwo\n\n[Bridge]\nThree\n";

    shekinah::ui::ShekinahWindow window;
    REQUIRE(window.open_project(project.root() / "Sunday"));
    const auto asset = window.import_asset_file(image_source);
    const auto song = window.import_song_file(song_source);
    REQUIRE(asset);
    REQUIRE(song);
    window.library_dock()->add_song(*song);
    window.library_dock()->show_songs();
    window.library_dock()->activate_song_row(0);
    auto* output = window.audience_output();
    window.show();
    QTest::qWait(20);

    auto* live = window.operator_page()->cue_grid()->findChild<QWidget*>(
        QStringLiteral("cue-%1").arg(QString::fromStdString(song->slides[1].id.value)));
    REQUIRE(live != nullptr);
    QTest::mouseClick(live, Qt::LeftButton, Qt::NoModifier, live->rect().center());
    REQUIRE(output->program().live_cue->cue == song->slides[1].id);

    auto* mime = shekinah::ui::make_asset_mime_data(asset->id);
    const auto position = test_support::drag_event_position(live->rect().center());
    QDragEnterEvent enter{position, Qt::CopyAction, mime, Qt::LeftButton, Qt::NoModifier};
    QApplication::sendEvent(live, &enter);
    QDragMoveEvent move{position, Qt::CopyAction, mime, Qt::LeftButton, Qt::NoModifier};
    QApplication::sendEvent(live, &move);
    QDropEvent drop{position, Qt::CopyAction, mime, Qt::LeftButton, Qt::NoModifier};
    QApplication::sendEvent(live, &drop);
    delete mime;

    REQUIRE(drop.isAccepted());
    QTest::qWait(20);
    REQUIRE(output->slide_image_loaded());
    const auto preview = window.operator_page()->program_status()->findChild<QWidget*>(
        QStringLiteral("program-preview"));
    REQUIRE(preview != nullptr);
    REQUIRE(preview->property("slide-background-path").toString() ==
            QString::fromStdString((project.root() / "Sunday" / asset->relative_path).string()));

    auto* following = window.operator_page()->cue_grid()->findChild<QWidget*>(
        QStringLiteral("cue-%1").arg(QString::fromStdString(song->slides[2].id.value)));
    REQUIRE(following != nullptr);
    REQUIRE(following->property("cue-background-path").toString().isEmpty());
    QTest::mouseClick(following, Qt::LeftButton, Qt::NoModifier, following->rect().center());
    REQUIRE(output->program().live_cue->cue == song->slides[2].id);
    REQUIRE(output->slide_image_loaded());
    REQUIRE(window.operator_page()->program_status()
                ->findChild<QWidget*>(QStringLiteral("program-preview"))
                ->property("slide-background-path")
                .toString() ==
            QString::fromStdString((project.root() / "Sunday" / asset->relative_path).string()));
}

TEST_CASE("forward Song slide backgrounds survive a project reopen") {
    ensure_qapplication();
    auto project = TestProject::create();
    const auto image_source = project.root() / "background.png";
    QImage image{800, 450, QImage::Format_RGB32};
    image.fill(Qt::darkGreen);
    REQUIRE(image.save(QString::fromStdString(image_source.string()), "PNG"));
    const auto song_source = project.root() / "goodness.txt";
    std::ofstream{song_source}
        << "# Goodness\n[Verse]\nOne\n\n[Chorus]\nTwo\n\n[Bridge]\nThree\n";

    shekinah::domain::SongId song_id;
    shekinah::domain::AssetId asset_id;
    {
        shekinah::ui::ShekinahWindow window;
        REQUIRE(window.open_project(project.root() / "Sunday"));
        const auto asset = window.import_asset_file(image_source);
        const auto song = window.import_song_file(song_source);
        REQUIRE(asset);
        REQUIRE(song);
        song_id = song->id;
        asset_id = asset->id;
        window.library_dock()->add_song(*song);
        window.library_dock()->show_songs();
        window.library_dock()->activate_song_row(0);
        window.show();
        QTest::qWait(20);

        auto* target = window.operator_page()->cue_grid()->findChild<QWidget*>(
            QStringLiteral("cue-%1").arg(QString::fromStdString(song->slides[1].id.value)));
        REQUIRE(target != nullptr);
        auto* mime = shekinah::ui::make_asset_mime_data(asset->id);
        const auto position = test_support::drag_event_position(target->rect().center());
        QDragEnterEvent enter{position, Qt::CopyAction, mime, Qt::LeftButton, Qt::NoModifier};
        QApplication::sendEvent(target, &enter);
        QDragMoveEvent move{position, Qt::CopyAction, mime, Qt::LeftButton, Qt::NoModifier};
        QApplication::sendEvent(target, &move);
        QDropEvent drop{position, Qt::CopyAction, mime, Qt::LeftButton, Qt::NoModifier};
        QApplication::sendEvent(target, &drop);
        delete mime;
        REQUIRE(drop.isAccepted());
        QTest::qWait(20);
    }

    shekinah::ui::ShekinahWindow reopened;
    REQUIRE(reopened.open_project(project.root() / "Sunday"));
    auto store = shekinah::storage::ProjectStore::create_or_open(project.root() / "Sunday");
    REQUIRE(store);
    shekinah::storage::SqliteContentRepository repository{*store};
    const auto saved_song = repository.load_song(song_id);
    REQUIRE(saved_song);
    REQUIRE_FALSE(saved_song->slides[0].background.has_value());
    REQUIRE(saved_song->slides[1].background->asset == asset_id);
    REQUIRE(saved_song->slides[2].background->asset == asset_id);
}
