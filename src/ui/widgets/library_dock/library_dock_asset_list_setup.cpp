#include "shekinah/ui/library_dock_widget.hpp"

#include "library_components.hpp"
#include "library_lists.hpp"

#include <QAbstractItemView>
#include <QListView>
#include <QStackedWidget>

namespace shekinah::ui {
using namespace library_internal;

void LibraryDockWidget::setup_asset_list() {
    assets_ = new AssetListWidget{stack_};
    assets_->setObjectName(QStringLiteral("assets-list"));
    assets_->setViewMode(QListView::IconMode);
    assets_->setFlow(QListView::LeftToRight);
    assets_->setWrapping(true);
    assets_->setResizeMode(QListView::Adjust);
    assets_->setMovement(QListView::Static);
    assets_->setSelectionMode(QAbstractItemView::SingleSelection);
    assets_->setDragEnabled(true);
    assets_->setDragDropMode(QAbstractItemView::DragOnly);
    assets_->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    assets_->setMouseTracking(true);
    assets_->setSpacing(4);
    assets_->setIconSize(QSize{kThumbnailWidth, kThumbnailHeight});
    assets_->setGridSize(QSize{kAssetGridWidth, kAssetGridHeight});
    assets_->setUniformItemSizes(true);
    assets_->setItemDelegate(make_asset_card_delegate(assets_));
}

}  // namespace shekinah::ui
