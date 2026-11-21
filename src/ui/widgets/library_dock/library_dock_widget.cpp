#include "shekinah/ui/library_dock_widget.hpp"

#include <QImageReader>
#include <QAbstractItemView>
#include <QIcon>
#include <QListView>
#include <QListWidget>
#include <QLineEdit>
#include <QMenu>
#include <QPixmap>
#include <QPushButton>
#include <QStyle>
#include <QTabWidget>
#include <QVBoxLayout>

namespace shekinah::ui {
namespace {

constexpr int kContentIdRole = Qt::UserRole + 1;

}  // namespace

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
    add_action(QStringLiteral("Edit Song"), QStringLiteral("edit-song"), [this] {
        if (edit_song_handler_) {
            edit_song_row(songs_->currentRow());
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
    add_action(QStringLiteral("Edit Sermon"), QStringLiteral("edit-sermon"), [this] {
        if (edit_sermon_handler_) {
            edit_sermon_row(sermons_->currentRow());
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
    assets_->setViewMode(QListView::IconMode);
    assets_->setFlow(QListView::LeftToRight);
    assets_->setWrapping(true);
    assets_->setResizeMode(QListView::Adjust);
    assets_->setMovement(QListView::Static);
    assets_->setSelectionMode(QAbstractItemView::SingleSelection);
    assets_->setDragEnabled(true);
    assets_->setDragDropMode(QAbstractItemView::DragOnly);
    assets_->setSpacing(10);
    assets_->setIconSize(QSize{168, 94});
    assets_->setGridSize(QSize{192, 126});
    assets_->setUniformItemSizes(true);
    songs_->setObjectName(QStringLiteral("songs-list"));
    sermons_->setObjectName(QStringLiteral("sermons-list"));
    songs_->setContextMenuPolicy(Qt::CustomContextMenu);
    QObject::connect(songs_, &QListWidget::customContextMenuRequested, this, [this](const QPoint& position) {
        auto* item = songs_->itemAt(position);
        if (item == nullptr || !edit_song_handler_) {
            return;
        }
        QMenu menu{this};
        auto* edit = menu.addAction(QStringLiteral("Edit Song"));
        QObject::connect(edit, &QAction::triggered, this, [this, item] {
            if (edit_song_handler_) {
                edit_song_handler_(domain::SongId{item->data(kContentIdRole).toString().toStdString()});
            }
        });
        menu.exec(songs_->viewport()->mapToGlobal(position));
    });
    QObject::connect(songs_, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem* item) {
        if (item != nullptr && add_content_handler_) {
            add_content_handler_(domain::ContentRef::song(item->data(kContentIdRole).toString().toStdString()));
        }
    });
    QObject::connect(sermons_, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem* item) {
        if (item != nullptr && add_content_handler_) {
            add_content_handler_(domain::ContentRef::sermon(item->data(kContentIdRole).toString().toStdString()));
        }
    });
    sermons_->setContextMenuPolicy(Qt::CustomContextMenu);
    QObject::connect(sermons_, &QListWidget::customContextMenuRequested, this, [this](const QPoint& position) {
        auto* item = sermons_->itemAt(position);
        if (item == nullptr || !edit_sermon_handler_) {
            return;
        }
        QMenu menu{this};
        auto* edit = menu.addAction(QStringLiteral("Edit Sermon"));
        QObject::connect(edit, &QAction::triggered, this, [this, item] {
            if (edit_sermon_handler_) {
                edit_sermon_handler_(domain::SermonId{item->data(kContentIdRole).toString().toStdString()});
            }
        });
        menu.exec(sermons_->viewport()->mapToGlobal(position));
    });
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
    const auto filename = std::filesystem::path{asset.relative_path}.filename().string();
    auto* item = new QListWidgetItem{
        QString::fromStdString(filename.empty() ? asset.relative_path : filename)};
    item->setData(kContentIdRole, QString::fromStdString(asset.id.value));
    item->setToolTip(QStringLiteral("%1\n%2 bytes")
                         .arg(QString::fromStdString(asset.relative_path))
                         .arg(static_cast<qulonglong>(asset.size_bytes)));
    if (asset.kind == domain::AssetKind::image && !asset_root_.empty()) {
        QImageReader reader{QString::fromStdString((asset_root_ / asset.relative_path).string())};
        reader.setAutoTransform(true);
        const auto source_size = reader.size();
        if (source_size.isValid()) {
            constexpr int kThumbnailWidth = 168;
            constexpr int kThumbnailHeight = 94;
            reader.setScaledSize(source_size.scaled(kThumbnailWidth, kThumbnailHeight, Qt::KeepAspectRatio));
        }
        const auto image = reader.read();
        if (!image.isNull()) {
            item->setIcon(QIcon{QPixmap::fromImage(image)});
        }
    } else if (asset.kind == domain::AssetKind::video) {
        item->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    }
    assets_->addItem(item);
    apply_filter();
}

void LibraryDockWidget::add_song(domain::Song song) {
    auto* item = new QListWidgetItem{QString::fromStdString(song.title)};
    item->setData(kContentIdRole, QString::fromStdString(song.id.value));
    songs_->addItem(item);
    apply_filter();
}

void LibraryDockWidget::add_sermon(domain::SermonDeck deck) {
    auto* item = new QListWidgetItem{QString::fromStdString(deck.title)};
    item->setData(kContentIdRole, QString::fromStdString(deck.id.value));
    sermons_->addItem(item);
    apply_filter();
}

void LibraryDockWidget::clear_contents() {
    assets_->clear();
    songs_->clear();
    sermons_->clear();
    apply_filter();
}

void LibraryDockWidget::set_search_text(const QString& text) { search_->setText(text); }

void LibraryDockWidget::activate_song_row(int row) {
    if (row >= 0 && row < songs_->count()) {
        const auto* item = songs_->item(row);
        if (add_content_handler_ && item != nullptr) {
            add_content_handler_(domain::ContentRef::song(item->data(kContentIdRole).toString().toStdString()));
        }
    }
}

void LibraryDockWidget::edit_song_row(int row) {
    if (row >= 0 && row < songs_->count()) {
        const auto* item = songs_->item(row);
        if (edit_song_handler_ && item != nullptr) {
            edit_song_handler_(domain::SongId{item->data(kContentIdRole).toString().toStdString()});
        }
    }
}

void LibraryDockWidget::activate_sermon_row(int row) {
    if (row >= 0 && row < sermons_->count()) {
        const auto* item = sermons_->item(row);
        if (add_content_handler_ && item != nullptr) {
            add_content_handler_(domain::ContentRef::sermon(item->data(kContentIdRole).toString().toStdString()));
        }
    }
}

void LibraryDockWidget::edit_sermon_row(int row) {
    if (row >= 0 && row < sermons_->count()) {
        const auto* item = sermons_->item(row);
        if (edit_sermon_handler_ && item != nullptr) {
            edit_sermon_handler_(domain::SermonId{item->data(kContentIdRole).toString().toStdString()});
        }
    }
}

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
