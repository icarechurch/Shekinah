#include "shekinah/ui/service_order_widget.hpp"

#include <QLineEdit>
#include <QListWidget>
#include <QMenu>
#include <QPushButton>

namespace shekinah::ui {

void ServiceOrderWidget::move_selected(int delta) {
    const int row = list_->currentRow();
    const int target = row + delta;
    if (row >= 0 && row < static_cast<int>(items_.size()) &&
        target >= 0 && target < static_cast<int>(items_.size()) &&
        move_handler_) {
        move_handler_(items_[static_cast<std::size_t>(row)].item.id, target);
    }
}

void ServiceOrderWidget::remove_selected() {
    const int row = list_->currentRow();
    if (row >= 0 && row < static_cast<int>(items_.size()) && remove_handler_) {
        remove_handler_(items_[static_cast<std::size_t>(row)].item.id);
    }
}

void ServiceOrderWidget::edit_selected() {
    const int row = list_->currentRow();
    if (row >= 0 && row < static_cast<int>(items_.size()) && edit_handler_) {
        edit_handler_(items_[static_cast<std::size_t>(row)].item.content);
    }
}

void ServiceOrderWidget::open_add_menu() {
    QMenu menu{this};
    if (new_song_handler_) {
        auto* action = menu.addAction(QStringLiteral("New Song..."));
        QObject::connect(action, &QAction::triggered, this, [this] { new_song_handler_(); });
    }
    if (import_song_handler_) {
        auto* action = menu.addAction(QStringLiteral("Import Song Lyrics..."));
        QObject::connect(action, &QAction::triggered, this, [this] { import_song_handler_(); });
    }
    if ((new_song_handler_ || import_song_handler_) &&
        (new_sermon_handler_ || import_sermon_handler_)) {
        menu.addSeparator();
    }
    if (new_sermon_handler_) {
        auto* action = menu.addAction(QStringLiteral("New Sermon..."));
        QObject::connect(action, &QAction::triggered, this, [this] { new_sermon_handler_(); });
    }
    if (import_sermon_handler_) {
        auto* action = menu.addAction(QStringLiteral("Import Sermon Deck..."));
        QObject::connect(action, &QAction::triggered, this, [this] { import_sermon_handler_(); });
    }
    if (menu.isEmpty()) {
        return;
    }
    menu.exec(add_button_->mapToGlobal(QPoint{0, add_button_->height()}));
}

void ServiceOrderWidget::open_context_menu(const QPoint& viewport_pos) {
    auto* clicked = list_->itemAt(viewport_pos);
    if (clicked == nullptr) {
        return;
    }
    const int row = list_->row(clicked);
    if (row < 0 || row >= static_cast<int>(items_.size())) {
        return;
    }
    list_->setCurrentRow(row);

    QMenu menu{this};
    const bool filtering = !filter_input_->text().trimmed().isEmpty();
    auto* up_action = menu.addAction(QStringLiteral("Move Up"));
    up_action->setEnabled(!filtering && row > 0);
    QObject::connect(up_action, &QAction::triggered, this, [this] { move_selected(-1); });

    auto* down_action = menu.addAction(QStringLiteral("Move Down"));
    down_action->setEnabled(!filtering && row + 1 < static_cast<int>(items_.size()));
    QObject::connect(down_action, &QAction::triggered, this, [this] { move_selected(1); });

    if (edit_handler_) {
        menu.addSeparator();
        const bool is_song =
            items_[static_cast<std::size_t>(row)].item.content.kind == domain::ContentKind::song;
        auto* edit_action = menu.addAction(is_song ? QStringLiteral("Edit Song...")
                                                   : QStringLiteral("Edit Sermon..."));
        QObject::connect(edit_action, &QAction::triggered, this, [this] { edit_selected(); });
    }

    menu.addSeparator();
    auto* remove_action = menu.addAction(QStringLiteral("Remove from Service Order"));
    QObject::connect(remove_action, &QAction::triggered, this, [this] { remove_selected(); });

    menu.exec(list_->viewport()->mapToGlobal(viewport_pos));
}

}  // namespace shekinah::ui
