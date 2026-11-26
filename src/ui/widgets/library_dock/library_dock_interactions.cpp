#include "shekinah/ui/library_dock_widget.hpp"

#include "library_components.hpp"

#include <QAction>
#include <QKeySequence>
#include <QLineEdit>
#include <QListWidget>
#include <QMenu>
#include <QShortcut>

namespace shekinah::ui {
using namespace library_internal;

void LibraryDockWidget::setup_interactions() {
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

    sermons_->setContextMenuPolicy(Qt::CustomContextMenu);
    QObject::connect(sermons_, &QListWidget::customContextMenuRequested, this, [this](const QPoint& position) {
        auto* item = sermons_->itemAt(position);
        if (item == nullptr) {
            return;
        }
        QMenu menu{this};
        if (add_content_handler_) {
            auto* add = menu.addAction(QStringLiteral("Add to Service Order"));
            QObject::connect(add, &QAction::triggered, this, [this, item] {
                if (add_content_handler_) {
                    add_content_handler_(
                        domain::ContentRef::sermon(item->data(kContentIdRole).toString().toStdString()));
                }
            });
        }
        if (edit_sermon_handler_) {
            auto* edit = menu.addAction(QStringLiteral("Edit Sermon"));
            QObject::connect(edit, &QAction::triggered, this, [this, item] {
                if (edit_sermon_handler_) {
                    edit_sermon_handler_(
                        domain::SermonId{item->data(kContentIdRole).toString().toStdString()});
                }
            });
        }
        if (!menu.isEmpty()) {
            menu.exec(sermons_->viewport()->mapToGlobal(position));
        }
    });
    QObject::connect(sermons_, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem* item) {
        if (item != nullptr && add_content_handler_) {
            add_content_handler_(
                domain::ContentRef::sermon(item->data(kContentIdRole).toString().toStdString()));
        }
    });

    QObject::connect(songs_, &QListWidget::currentRowChanged, this, [this](int) {
        update_action_states();
    });
    QObject::connect(sermons_, &QListWidget::currentRowChanged, this, [this](int) {
        update_action_states();
    });

    auto* focus_search = new QShortcut{QKeySequence::Find, this};
    focus_search->setContext(Qt::WidgetWithChildrenShortcut);
    QObject::connect(focus_search, &QShortcut::activated, this, [this] {
        search_->setFocus(Qt::ShortcutFocusReason);
        search_->selectAll();
    });
    QObject::connect(search_, &QLineEdit::textChanged, this, [this] { apply_filter(); });
}

}  // namespace shekinah::ui
