#include "shekinah/ui/library_dock_widget.hpp"
#include "library_components.hpp"
#include "shekinah/ui/support/asset_drag.hpp"
#include "shekinah/ui/support/content_drag.hpp"
#include "shekinah/ui/support/elided_label.hpp"
#include "shekinah/ui/support/flow_layout.hpp"
#include "shekinah/ui/support/theme.hpp"

#include <QAbstractItemView>
#include <QAction>
#include <QButtonGroup>
#include <QFrame>
#include <QHBoxLayout>
#include <QIcon>
#include <QImageReader>
#include <QKeyEvent>
#include <QKeySequence>
#include <QLabel>
#include <QLineEdit>
#include <QListView>
#include <QListWidget>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QPushButton>
#include <QResizeEvent>
#include <QShortcut>
#include <QStackedWidget>
#include <QStyle>
#include <QStyledItemDelegate>
#include <QVBoxLayout>

#include <algorithm>
#include <utility>

namespace shekinah::ui {
using namespace library_internal;
using namespace library_internal;
void LibraryDockWidget::show_assets() { set_tab(LibraryTab::assets, 0); }

void LibraryDockWidget::show_songs() { set_tab(LibraryTab::songs, 1); }

void LibraryDockWidget::show_sermons() { set_tab(LibraryTab::sermons, 2); }

void LibraryDockWidget::set_asset_root(std::filesystem::path root) {
    if (asset_root_ != root) {
        thumbnail_cache_.clear();
    }
    asset_root_ = std::move(root);
}

void LibraryDockWidget::add_asset(domain::AssetRecord asset) {
    const auto filename = std::filesystem::path{asset.relative_path}.filename().string();
    auto* item = new QListWidgetItem{
        QString::fromStdString(filename.empty() ? asset.relative_path : filename)};
    item->setData(kContentIdRole, QString::fromStdString(asset.id.value));
    item->setData(kAssetKindRole, static_cast<int>(asset.kind));
    item->setData(kAssetSizeRole, static_cast<qulonglong>(asset.size_bytes));
    item->setData(kRelativePathRole, QString::fromStdString(asset.relative_path));
    item->setToolTip(QStringLiteral("%1\n%2 (%3 bytes)\nDrag onto a Song or Slide to set background")
                         .arg(QString::fromStdString(asset.relative_path))
                         .arg(format_compact_bytes(asset.size_bytes))
                         .arg(static_cast<qulonglong>(asset.size_bytes)));
    if (asset.kind == domain::AssetKind::image && !asset_root_.empty()) {
        const QIcon thumbnail = load_bounded_thumbnail(asset);
        if (!thumbnail.isNull()) {
            item->setIcon(thumbnail);
        }
    } else if (asset.kind == domain::AssetKind::video) {
        item->setIcon(shared_video_icon_);
    }
    assets_->addItem(item);
    apply_filter_to_asset(item, search_->text());
    update_tab_and_header_state();
    update_empty_state();
}

void LibraryDockWidget::add_song(domain::Song song) {
    const QString id = QString::fromStdString(song.id.value);
    QListWidgetItem* item = nullptr;
    for (int index = 0; index != songs_->count(); ++index) {
        if (songs_->item(index)->data(kContentIdRole).toString() == id) {
            item = songs_->item(index);
            break;
        }
    }
    if (item == nullptr) {
        item = new QListWidgetItem{};
        item->setData(kContentIdRole, id);
        songs_->addItem(item);
    }
    item->setText(QString::fromStdString(song.title));
    item->setData(kContentKindRole, static_cast<int>(domain::ContentKind::song));
    item->setData(kSlideCountRole, static_cast<int>(song.slides.size()));
    item->setData(kHasBgRole, song.default_background.has_value());
    item->setToolTip(QStringLiteral("%1\n%2 slide%3 Â· Double-click or drag to Service Order")
                         .arg(item->text())
                         .arg(song.slides.size())
                         .arg(song.slides.size() == 1 ? QString() : QStringLiteral("s")));
    apply_filter_to_content(item, search_->text());
    update_tab_and_header_state();
    update_empty_state();
}

void LibraryDockWidget::add_sermon(domain::SermonDeck deck) {
    const QString id = QString::fromStdString(deck.id.value);
    QListWidgetItem* item = nullptr;
    for (int index = 0; index != sermons_->count(); ++index) {
        if (sermons_->item(index)->data(kContentIdRole).toString() == id) {
            item = sermons_->item(index);
            break;
        }
    }
    if (item == nullptr) {
        item = new QListWidgetItem{};
        item->setData(kContentIdRole, id);
        sermons_->addItem(item);
    }
    item->setText(QString::fromStdString(deck.title));
    item->setData(kContentKindRole, static_cast<int>(domain::ContentKind::sermon));
    item->setData(kSlideCountRole, static_cast<int>(deck.slides.size()));
    item->setData(kHasBgRole, false);
    item->setToolTip(QStringLiteral("%1\n%2 slide%3 Â· Double-click or drag to Service Order")
                         .arg(item->text())
                         .arg(deck.slides.size())
                         .arg(deck.slides.size() == 1 ? QString() : QStringLiteral("s")));
    apply_filter_to_content(item, search_->text());
    update_tab_and_header_state();
    update_empty_state();
}

void LibraryDockWidget::clear_contents() {
    assets_->clear();
    songs_->clear();
    sermons_->clear();
    update_tab_and_header_state();
    update_action_states();
    update_empty_state();
}

void LibraryDockWidget::purge_non_live_caches() {
    thumbnail_cache_.clear();
}

void LibraryDockWidget::set_search_text(const QString& text) { search_->setText(text); }

void LibraryDockWidget::set_asset_kind_filter(std::optional<domain::AssetKind> kind) {
    asset_kind_filter_ = kind;
    if (!kind.has_value()) {
        filter_all_button_->setChecked(true);
    } else if (*kind == domain::AssetKind::image) {
        filter_images_button_->setChecked(true);
    } else {
        filter_videos_button_->setChecked(true);
    }
    apply_filter();
}

void LibraryDockWidget::activate_song_row(int row) {
    if (row >= 0 && row < songs_->count()) {
        const auto* item = songs_->item(row);
        if (add_content_handler_ && item != nullptr) {
            add_content_handler_(
                domain::ContentRef::song(item->data(kContentIdRole).toString().toStdString()));
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
            add_content_handler_(
                domain::ContentRef::sermon(item->data(kContentIdRole).toString().toStdString()));
        }
    }
}

void LibraryDockWidget::edit_sermon_row(int row) {
    if (row >= 0 && row < sermons_->count()) {
        const auto* item = sermons_->item(row);
        if (edit_sermon_handler_ && item != nullptr) {
            edit_sermon_handler_(
                domain::SermonId{item->data(kContentIdRole).toString().toStdString()});
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

void LibraryDockWidget::resizeEvent(QResizeEvent* event) {
    QDockWidget::resizeEvent(event);
    layout_empty_overlay();
    update_responsive_grid_sizes();
}

}  // namespace shekinah::ui
