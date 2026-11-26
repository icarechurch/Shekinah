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

TEST_CASE("assets dragged before first slide, between slides, and after last slide insert Song media slides") {
    ensure_qapplication();
    auto project = TestProject::create();
    const auto intro_source = project.root() / "intro.png";
    const auto interlude_source = project.root() / "interlude.png";
    const auto outro_source = project.root() / "outro.png";
    for (const auto& [path, color] : {std::pair{intro_source, Qt::red},
                                      std::pair{interlude_source, Qt::blue},
                                      std::pair{outro_source, Qt::green}}) {
        QImage image{800, 450, QImage::Format_RGB32};
        image.fill(color);
        REQUIRE(image.save(QString::fromStdString(path.string()), "PNG"));
    }
    const auto song_source = project.root() / "goodness.txt";
    std::ofstream{song_source} << "# Goodness\n[Verse]\nOne\n\n[Chorus]\nTwo\n";

    shekinah::domain::SongId song_id;
    shekinah::domain::AssetId intro_id;
    shekinah::domain::AssetId interlude_id;
    shekinah::domain::AssetId outro_id;
    {
        shekinah::ui::ShekinahWindow window;
        REQUIRE(window.open_project(project.root() / "Sunday"));
        const auto intro = window.import_asset_file(intro_source);
        const auto interlude = window.import_asset_file(interlude_source);
        const auto outro = window.import_asset_file(outro_source);
        const auto song = window.import_song_file(song_source);
        REQUIRE(intro);
        REQUIRE(interlude);
        REQUIRE(outro);
        REQUIRE(song);
        song_id = song->id;
        intro_id = intro->id;
        interlude_id = interlude->id;
        outro_id = outro->id;
        window.library_dock()->add_song(*song);
        window.library_dock()->show_songs();
        window.library_dock()->activate_song_row(0);
        window.resize(1200, 720);
        window.show();
        QTest::qWait(30);

        auto* grid = window.operator_page()->cue_grid();
        auto* container = grid->findChild<QWidget*>(QStringLiteral("cue-grid-container"));
        REQUIRE(container != nullptr);

        const auto drop_in_container = [&](QPoint pt, const shekinah::domain::AssetId& asset_id) {
            auto* mime = shekinah::ui::make_asset_mime_data(asset_id);
            const auto pos = test_support::drag_event_position(pt);
            QDragEnterEvent enter{pos, Qt::CopyAction, mime, Qt::LeftButton, Qt::NoModifier};
            QApplication::sendEvent(container, &enter);
            REQUIRE(enter.isAccepted());
            QDropEvent drop{pos, Qt::CopyAction, mime, Qt::LeftButton, Qt::NoModifier};
            QApplication::sendEvent(container, &drop);
            REQUIRE(drop.isAccepted());
            delete mime;
            QTest::qWait(20);
        };

        // Drop before the first slide
        auto* slide0 = grid->findChild<QWidget*>(
            QStringLiteral("cue-%1").arg(QString::fromStdString(song->slides[0].id.value)));
        REQUIRE(slide0 != nullptr);
        drop_in_container(QPoint{std::max(1, slide0->x() / 2), slide0->geometry().center().y()},
                          intro->id);

        // Drop between the two lyric slides (now at indices 1 and 2)
        auto* lyric1 = grid->findChild<QWidget*>(
            QStringLiteral("cue-%1").arg(QString::fromStdString(song->slides[0].id.value)));
        auto* lyric2 = grid->findChild<QWidget*>(
            QStringLiteral("cue-%1").arg(QString::fromStdString(song->slides[1].id.value)));
        REQUIRE(lyric1 != nullptr);
        REQUIRE(lyric2 != nullptr);
        drop_in_container(
            QPoint{(lyric1->geometry().right() + 1 + lyric2->x()) / 2, lyric1->geometry().center().y()},
            interlude->id);

        // Drop after the last slide
        lyric2 = grid->findChild<QWidget*>(
            QStringLiteral("cue-%1").arg(QString::fromStdString(song->slides[1].id.value)));
        REQUIRE(lyric2 != nullptr);
        drop_in_container(
            QPoint{lyric2->geometry().right() + 6, lyric2->geometry().center().y()},
            outro->id);

        REQUIRE(window.operator_page()->service_order_widget()->items()[0].slide_count == 5);
    }

    auto store = shekinah::storage::ProjectStore::create_or_open(project.root() / "Sunday");
    REQUIRE(store);
    shekinah::storage::SqliteContentRepository repository{*store};
    const auto saved_song = repository.load_song(song_id);
    REQUIRE(saved_song);
    REQUIRE(saved_song->slides.size() == 5);
    REQUIRE(saved_song->slides[0].lines.empty());
    REQUIRE(saved_song->slides[0].background->asset == intro_id);
    REQUIRE(saved_song->slides[1].lines == std::vector<std::string>{"One"});
    REQUIRE_FALSE(saved_song->slides[1].background.has_value());
    REQUIRE(saved_song->slides[2].lines.empty());
    REQUIRE(saved_song->slides[2].background->asset == interlude_id);
    REQUIRE(saved_song->slides[3].lines == std::vector<std::string>{"Two"});
    REQUIRE_FALSE(saved_song->slides[3].background.has_value());
    REQUIRE(saved_song->slides[4].lines.empty());
    REQUIRE(saved_song->slides[4].background->asset == outro_id);
}
TEST_CASE("assets dragged before first slide and between slides insert Sermon slides") {
    ensure_qapplication();
    auto project = TestProject::create();
    const auto slide_a = project.root() / "a.png";
    const auto slide_b = project.root() / "b.png";
    const auto extra_before = project.root() / "before.png";
    const auto extra_between = project.root() / "between.png";
    for (const auto& [path, color] : {std::pair{slide_a, Qt::cyan},
                                      std::pair{slide_b, Qt::yellow},
                                      std::pair{extra_before, Qt::red},
                                      std::pair{extra_between, Qt::magenta}}) {
        QImage image{800, 450, QImage::Format_RGB32};
        image.fill(color);
        REQUIRE(image.save(QString::fromStdString(path.string()), "PNG"));
    }

    shekinah::ui::ShekinahWindow window;
    REQUIRE(window.open_project(project.root() / "Sunday"));
    const auto sermon = window.import_sermon_files({slide_a, slide_b}, "Sunday Sermon", true);
    const auto asset_before = window.import_asset_file(extra_before);
    const auto asset_between = window.import_asset_file(extra_between);
    REQUIRE(sermon);
    REQUIRE(asset_before);
    REQUIRE(asset_between);
    window.library_dock()->add_sermon(*sermon);
    window.library_dock()->show_sermons();
    window.library_dock()->activate_sermon_row(0);
    window.resize(1200, 720);
    window.show();
    QTest::qWait(30);

    auto* grid = window.operator_page()->cue_grid();
    auto* container = grid->findChild<QWidget*>(QStringLiteral("cue-grid-container"));
    REQUIRE(container != nullptr);

    auto* first_card = grid->findChild<QWidget*>(
        QStringLiteral("cue-%1").arg(QString::fromStdString(sermon->slides[0].id.value)));
    REQUIRE(first_card != nullptr);
    {
        auto* mime = shekinah::ui::make_asset_mime_data(asset_before->id);
        const auto pos = test_support::drag_event_position(
            QPoint{std::max(1, first_card->x() / 2), first_card->geometry().center().y()});
        QDragEnterEvent enter{pos, Qt::CopyAction, mime, Qt::LeftButton, Qt::NoModifier};
        QApplication::sendEvent(container, &enter);
        QDropEvent drop{pos, Qt::CopyAction, mime, Qt::LeftButton, Qt::NoModifier};
        QApplication::sendEvent(container, &drop);
        delete mime;
        REQUIRE(drop.isAccepted());
        QTest::qWait(20);
    }

    first_card = grid->findChild<QWidget*>(
        QStringLiteral("cue-%1").arg(QString::fromStdString(sermon->slides[0].id.value)));
    auto* second_card = grid->findChild<QWidget*>(
        QStringLiteral("cue-%1").arg(QString::fromStdString(sermon->slides[1].id.value)));
    REQUIRE(first_card != nullptr);
    REQUIRE(second_card != nullptr);
    {
        auto* mime = shekinah::ui::make_asset_mime_data(asset_between->id);
        const auto pos = test_support::drag_event_position(
            QPoint{(first_card->geometry().right() + 1 + second_card->x()) / 2,
                   first_card->geometry().center().y()});
        QDragEnterEvent enter{pos, Qt::CopyAction, mime, Qt::LeftButton, Qt::NoModifier};
        QApplication::sendEvent(container, &enter);
        QDropEvent drop{pos, Qt::CopyAction, mime, Qt::LeftButton, Qt::NoModifier};
        QApplication::sendEvent(container, &drop);
        delete mime;
        REQUIRE(drop.isAccepted());
        QTest::qWait(20);
    }

    auto store = shekinah::storage::ProjectStore::create_or_open(project.root() / "Sunday");
    REQUIRE(store);
    shekinah::storage::SqliteContentRepository repository{*store};
    const auto saved_sermon = repository.load_deck(sermon->id);
    REQUIRE(saved_sermon);
    REQUIRE(saved_sermon->slides.size() == 4);
    REQUIRE(saved_sermon->slides[0].image == asset_before->id);
    REQUIRE(saved_sermon->slides[1].id == sermon->slides[0].id);
    REQUIRE(saved_sermon->slides[2].image == asset_between->id);
    REQUIRE(saved_sermon->slides[3].id == sermon->slides[1].id);
    REQUIRE(window.operator_page()->service_order_widget()->items()[0].slide_count == 4);

    // Click the newly inserted first sermon slide to verify it goes live cleanly
    auto* inserted_first = grid->findChild<QWidget*>(
        QStringLiteral("cue-%1").arg(QString::fromStdString(saved_sermon->slides[0].id.value)));
    REQUIRE(inserted_first != nullptr);
    QTest::mouseClick(inserted_first, Qt::LeftButton, Qt::NoModifier, inserted_first->rect().center());
    REQUIRE(window.audience_output()->program().live_cue->cue == saved_sermon->slides[0].id);
    REQUIRE(window.audience_output()->slide_image_loaded());
}

