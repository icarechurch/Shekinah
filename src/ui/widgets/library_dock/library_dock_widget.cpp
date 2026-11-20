#include "shekinah/ui/library_dock_widget.hpp"

#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QTabWidget>
#include <QVBoxLayout>

namespace shekinah::ui {

LibraryDockWidget::LibraryDockWidget(QWidget* parent) : QDockWidget(parent) {
    setObjectName(QStringLiteral("library-dock"));
    setWindowTitle(QStringLiteral("Library"));
    setAllowedAreas(Qt::BottomDockWidgetArea | Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable |
                QDockWidget::DockWidgetClosable);

    auto* container = new QWidget{this};
    auto* container_layout = new QVBoxLayout(container);
    container_layout->setContentsMargins(0, 0, 0, 0);
    search_ = new QLineEdit{container};
    search_->setObjectName(QStringLiteral("library-search"));
    search_->setPlaceholderText(QStringLiteral("Search Assets, Songs, or Sermons"));
    container_layout->addWidget(search_);
    auto* actions = new QWidget{container};
    auto* actions_layout = new QHBoxLayout(actions);
    actions_layout->setContentsMargins(0, 0, 0, 0);
    const auto add_action = [&](const QString& label, const QString& object_name, auto handler) {
        auto* button = new QPushButton{label, actions};
        button->setObjectName(object_name);
        QObject::connect(button, &QPushButton::clicked, this, handler);
        actions_layout->addWidget(button);
    };
    add_action(QStringLiteral("New Song"), QStringLiteral("new-song"), [this] {
        if (new_song_handler_) {
            new_song_handler_();
        }
    });
    add_action(QStringLiteral("Import Song Text"), QStringLiteral("import-song-text"), [this] {
        if (import_song_handler_) {
            import_song_handler_();
        }
    });
    add_action(QStringLiteral("New Sermon"), QStringLiteral("new-sermon"), [this] {
        if (new_sermon_handler_) {
            new_sermon_handler_();
        }
    });
    add_action(QStringLiteral("Import Sermon Deck"), QStringLiteral("import-sermon-deck"), [this] {
        if (import_sermon_handler_) {
            import_sermon_handler_();
        }
    });
    add_action(QStringLiteral("Import Asset"), QStringLiteral("import-asset"), [this] {
        if (import_asset_handler_) {
            import_asset_handler_();
        }
    });
    container_layout->addWidget(actions);
    tabs_ = new QTabWidget{container};
    assets_ = new QListWidget{tabs_};
    songs_ = new QListWidget{tabs_};
    sermons_ = new QListWidget{tabs_};
    assets_->setObjectName(QStringLiteral("assets-list"));
    songs_->setObjectName(QStringLiteral("songs-list"));
    sermons_->setObjectName(QStringLiteral("sermons-list"));
    tabs_->addTab(assets_, QStringLiteral("Assets"));
    tabs_->addTab(songs_, QStringLiteral("Songs"));
    tabs_->addTab(sermons_, QStringLiteral("Sermons"));
    container_layout->addWidget(tabs_);
    setWidget(container);
    QObject::connect(search_, &QLineEdit::textChanged, this, [this] { apply_filter(); });
}

void LibraryDockWidget::show_assets() { set_tab(LibraryTab::assets, 0); }

void LibraryDockWidget::show_songs() { set_tab(LibraryTab::songs, 1); }

void LibraryDockWidget::show_sermons() { set_tab(LibraryTab::sermons, 2); }

void LibraryDockWidget::add_asset(domain::AssetRecord asset) {
    assets_->addItem(QString::fromStdString(asset.relative_path));
    apply_filter();
}

void LibraryDockWidget::add_song(domain::Song song) {
    songs_->addItem(QString::fromStdString(song.title));
    apply_filter();
}

void LibraryDockWidget::add_sermon(domain::SermonDeck deck) {
    sermons_->addItem(QString::fromStdString(deck.title));
    apply_filter();
}

void LibraryDockWidget::set_search_text(const QString& text) { search_->setText(text); }

int LibraryDockWidget::visible_asset_count() const noexcept {
    if (current_tab_ != LibraryTab::assets) {
        return 0;
    }
    int visible = 0;
    for (int index = 0; index != assets_->count(); ++index) {
        visible += assets_->item(index)->isHidden() ? 0 : 1;
    }
    return visible;
}

int LibraryDockWidget::visible_content_count() const noexcept {
    if (current_tab_ == LibraryTab::songs) {
        int visible = 0;
        for (int index = 0; index != songs_->count(); ++index) {
            visible += songs_->item(index)->isHidden() ? 0 : 1;
        }
        return visible;
    }
    if (current_tab_ == LibraryTab::sermons) {
        int visible = 0;
        for (int index = 0; index != sermons_->count(); ++index) {
            visible += sermons_->item(index)->isHidden() ? 0 : 1;
        }
        return visible;
    }
    return 0;
}

void LibraryDockWidget::set_tab(LibraryTab tab, int index) {
    current_tab_ = tab;
    tabs_->setCurrentIndex(index);
}

void LibraryDockWidget::apply_filter() {
    const auto query = search_->text();
    const auto filter = [&](QListWidget* list) {
        for (int index = 0; index != list->count(); ++index) {
            list->item(index)->setHidden(!query.isEmpty() &&
                                         !list->item(index)->text().contains(query, Qt::CaseInsensitive));
        }
    };
    filter(assets_);
    filter(songs_);
    filter(sermons_);
}

}  // namespace shekinah::ui
