#include "shekinah/ui/service_order_widget.hpp"

#include <QLineEdit>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>

namespace shekinah::ui {

void ServiceOrderWidget::set_authoring_handlers(std::function<void()> new_song,
                                                std::function<void()> import_song,
                                                std::function<void()> new_sermon,
                                                std::function<void()> import_sermon) {
    new_song_handler_ = std::move(new_song);
    import_song_handler_ = std::move(import_song);
    new_sermon_handler_ = std::move(new_sermon);
    import_sermon_handler_ = std::move(import_sermon);
    const bool any = static_cast<bool>(new_song_handler_) ||
                     static_cast<bool>(import_song_handler_) ||
                     static_cast<bool>(new_sermon_handler_) ||
                     static_cast<bool>(import_sermon_handler_);
    add_button_->setEnabled(any);
}

void ServiceOrderWidget::select_row(int row) {
    list_->setCurrentRow(row);
    update_controls_state();
}

int ServiceOrderWidget::count() const noexcept { return list_->count(); }

void ServiceOrderWidget::update_controls_state() {
    const int row = list_->currentRow();
    const bool valid = row >= 0 && row < static_cast<int>(items_.size());
    const bool filtering = !filter_input_->text().trimmed().isEmpty();
    move_up_button_->setEnabled(valid && !filtering && row > 0);
    move_down_button_->setEnabled(valid && !filtering && row + 1 < static_cast<int>(items_.size()));
    edit_button_->setEnabled(valid && static_cast<bool>(edit_handler_));
    remove_button_->setEnabled(valid);
}

}  // namespace shekinah::ui
