#include "shekinah/ui/library_dock_widget.hpp"

#include "library_components.hpp"

#include <QIcon>
#include <QLineEdit>
#include <QListWidget>

#include <filesystem>
#include <utility>

namespace shekinah::ui {
using namespace library_internal;

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
    item->setToolTip(QStringLiteral("%1\n%2 slide%3 Ã‚Â· Double-click or drag to Service Order")
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
    item->setToolTip(QStringLiteral("%1\n%2 slide%3 Ã‚Â· Double-click or drag to Service Order")
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

void LibraryDockWidget::purge_non_live_caches() { thumbnail_cache_.clear(); }

}  // namespace shekinah::ui
