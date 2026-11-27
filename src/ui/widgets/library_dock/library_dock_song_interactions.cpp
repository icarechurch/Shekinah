#include "shekinah/ui/library_dock_widget.hpp"

#include "library_components.hpp"

#include <QAction>
#include <QListWidget>
#include <QMenu>

namespace shekinah::ui {
using namespace library_internal;

void LibraryDockWidget::setup_song_interactions() {
    songs_->setContextMenuPolicy(Qt::CustomContextMenu);
    QObject::connect(songs_, &QListWidget::customContextMenuRequested, this, [this](const QPoint& position) {
        auto* item = songs_->itemAt(position);
        if (item == nullptr) {
            return;
        }
        QMenu menu{this};
        if (add_content_handler_) {
            auto* add = menu.addAction(QStringLiteral("Add to Service Order"));
            QObject::connect(add, &QAction::triggered, this, [this, item] {
                if (add_content_handler_) {
                    add_content_handler_(
                        domain::ContentRef::song(item->data(kContentIdRole).toString().toStdString()));
                }
            });
        }
        if (edit_song_handler_) {
            auto* edit = menu.addAction(QStringLiteral("Edit Song"));
            QObject::connect(edit, &QAction::triggered, this, [this, item] {
                if (edit_song_handler_) {
                    edit_song_handler_(
                        domain::SongId{item->data(kContentIdRole).toString().toStdString()});
                }
            });
        }
        if (!menu.isEmpty()) {
            menu.exec(songs_->viewport()->mapToGlobal(position));
        }
    });
    QObject::connect(songs_, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem* item) {
        if (item != nullptr && add_content_handler_) {
            add_content_handler_(
                domain::ContentRef::song(item->data(kContentIdRole).toString().toStdString()));
        }
    });
    QObject::connect(songs_, &QListWidget::currentRowChanged, this, [this](int) {
        update_action_states();
    });
}

}  // namespace shekinah::ui
