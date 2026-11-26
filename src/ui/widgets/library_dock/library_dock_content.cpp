#include "shekinah/ui/library_dock_widget.hpp"

#include "library_components.hpp"
#include "library_lists.hpp"

namespace shekinah::ui {
using namespace library_internal;

void LibraryDockWidget::setup_content(QWidget* container) {
    stack_ = new QStackedWidget{container};
    stack_->setObjectName(QStringLiteral("library-stack"));

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
    assets_->setItemDelegate(new AssetCardDelegate{assets_});

    auto* songs_list = new ContentListWidget{domain::ContentKind::song, stack_};
    auto* sermons_list = new ContentListWidget{domain::ContentKind::sermon, stack_};
    songs_ = songs_list;
    sermons_ = sermons_list;
    songs_->setObjectName(QStringLiteral("songs-list"));
    sermons_->setObjectName(QStringLiteral("sermons-list"));
    for (auto* list : {songs_, sermons_}) {
        list->setViewMode(QListView::ListMode);
        list->setFlow(QListView::LeftToRight);
        list->setWrapping(true);
        list->setResizeMode(QListView::Adjust);
        list->setMovement(QListView::Static);
        list->setSelectionMode(QAbstractItemView::SingleSelection);
        list->setDragEnabled(true);
        list->setDragDropMode(QAbstractItemView::DragOnly);
        list->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
        list->setMouseTracking(true);
        list->setSpacing(4);
        list->setGridSize(QSize{kContentGridWidth, kContentGridHeight});
        list->setUniformItemSizes(true);
        list->setItemDelegate(new ContentCardDelegate{list});
    }

    songs_list->set_action_handlers(
        [this](QListWidgetItem* item) {
            if (item != nullptr && add_content_handler_) {
                add_content_handler_(
                    domain::ContentRef::song(item->data(kContentIdRole).toString().toStdString()));
            }
        },
        [this](QListWidgetItem* item) {
            if (item != nullptr && edit_song_handler_) {
                edit_song_handler_(domain::SongId{item->data(kContentIdRole).toString().toStdString()});
            }
        });
    sermons_list->set_action_handlers(
        [this](QListWidgetItem* item) {
            if (item != nullptr && add_content_handler_) {
                add_content_handler_(
                    domain::ContentRef::sermon(item->data(kContentIdRole).toString().toStdString()));
            }
        },
        [this](QListWidgetItem* item) {
            if (item != nullptr && edit_sermon_handler_) {
                edit_sermon_handler_(
                    domain::SermonId{item->data(kContentIdRole).toString().toStdString()});
            }
        });

    stack_->addWidget(assets_);
    stack_->addWidget(songs_);
    stack_->addWidget(sermons_);
    static_cast<QVBoxLayout*>(container->layout())->addWidget(stack_, 1);

    empty_overlay_ = new QWidget{stack_};
    empty_overlay_->setObjectName(QStringLiteral("library-empty-state"));
    empty_overlay_->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    auto* empty_layout = new QVBoxLayout(empty_overlay_);
    empty_layout->setContentsMargins(24, 12, 24, 12);
    empty_layout->setSpacing(4);
    empty_layout->addStretch(1);
    empty_icon_label_ = new QLabel{empty_overlay_};
    empty_icon_label_->setAlignment(Qt::AlignCenter);
    empty_layout->addWidget(empty_icon_label_);
    empty_title_label_ = new QLabel{empty_overlay_};
    empty_title_label_->setObjectName(QStringLiteral("library-empty-title"));
    empty_title_label_->setAlignment(Qt::AlignCenter);
    empty_layout->addWidget(empty_title_label_);
    empty_hint_label_ = new QLabel{empty_overlay_};
    empty_hint_label_->setObjectName(QStringLiteral("library-empty-hint"));
    empty_hint_label_->setAlignment(Qt::AlignCenter);
    empty_hint_label_->setWordWrap(true);
    empty_layout->addWidget(empty_hint_label_);
    empty_layout->addStretch(1);
}

}  // namespace shekinah::ui
