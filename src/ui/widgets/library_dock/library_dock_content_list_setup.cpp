#include "shekinah/ui/library_dock_widget.hpp"

#include "library_components.hpp"
#include "library_lists.hpp"

#include <QAbstractItemView>
#include <QListView>
#include <QListWidget>
#include <QStackedWidget>

namespace shekinah::ui {
using namespace library_internal;

void LibraryDockWidget::setup_content_lists() {
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
        list->setItemDelegate(make_content_card_delegate(list));
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
}

}  // namespace shekinah::ui
