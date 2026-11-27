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

}  // namespace shekinah::ui
