#include "shekinah/ui/library_dock_widget.hpp"

#include "library_components.hpp"

#include <QAction>
#include <QListWidget>
#include <QMenu>

namespace shekinah::ui {
using namespace library_internal;

void LibraryDockWidget::setup_sermon_interactions() {
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
    QObject::connect(sermons_, &QListWidget::currentRowChanged, this, [this](int) {
        update_action_states();
    });
}

}  // namespace shekinah::ui
