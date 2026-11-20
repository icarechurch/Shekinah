#include "shekinah/ui/library_dock_widget.hpp"

#include <QListWidget>
#include <QTabWidget>

namespace shekinah::ui {

LibraryDockWidget::LibraryDockWidget(QWidget* parent) : QDockWidget(parent) {
    setObjectName(QStringLiteral("library-dock"));
    setWindowTitle(QStringLiteral("Library"));
    setAllowedAreas(Qt::BottomDockWidgetArea | Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable |
                QDockWidget::DockWidgetClosable);

    tabs_ = new QTabWidget{this};
    assets_ = new QListWidget{tabs_};
    songs_ = new QListWidget{tabs_};
    sermons_ = new QListWidget{tabs_};
    assets_->setObjectName(QStringLiteral("assets-list"));
    songs_->setObjectName(QStringLiteral("songs-list"));
    sermons_->setObjectName(QStringLiteral("sermons-list"));
    tabs_->addTab(assets_, QStringLiteral("Assets"));
    tabs_->addTab(songs_, QStringLiteral("Songs"));
    tabs_->addTab(sermons_, QStringLiteral("Sermons"));
    setWidget(tabs_);
}

void LibraryDockWidget::show_assets() { set_tab(LibraryTab::assets, 0); }

void LibraryDockWidget::show_songs() { set_tab(LibraryTab::songs, 1); }

void LibraryDockWidget::show_sermons() { set_tab(LibraryTab::sermons, 2); }

void LibraryDockWidget::add_asset(domain::AssetRecord asset) {
    assets_->addItem(QString::fromStdString(asset.relative_path));
}

void LibraryDockWidget::add_song(domain::Song song) {
    songs_->addItem(QString::fromStdString(song.title));
}

void LibraryDockWidget::add_sermon(domain::SermonDeck deck) {
    sermons_->addItem(QString::fromStdString(deck.title));
}

int LibraryDockWidget::visible_asset_count() const noexcept {
    return current_tab_ == LibraryTab::assets ? assets_->count() : 0;
}

int LibraryDockWidget::visible_content_count() const noexcept {
    if (current_tab_ == LibraryTab::songs) {
        return songs_->count();
    }
    if (current_tab_ == LibraryTab::sermons) {
        return sermons_->count();
    }
    return 0;
}

void LibraryDockWidget::set_tab(LibraryTab tab, int index) {
    current_tab_ = tab;
    tabs_->setCurrentIndex(index);
}

}  // namespace shekinah::ui
