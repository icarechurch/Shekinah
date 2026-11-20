#include "shekinah/ui/service_order_widget.hpp"

#include <QListWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>

namespace shekinah::ui {

ServiceOrderWidget::ServiceOrderWidget(QWidget* parent) : QWidget(parent) {
    setObjectName(QStringLiteral("service-order"));
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    list_ = new QListWidget{this};
    list_->setObjectName(QStringLiteral("service-order-list"));
    QObject::connect(list_, &QListWidget::currentRowChanged, this, [this](int row) {
        if (row >= 0 && row < static_cast<int>(items_.size()) && selection_handler_) {
            selection_handler_(items_[static_cast<std::size_t>(row)].item.id);
        }
    });
    layout->addWidget(list_);

    auto* controls = new QHBoxLayout;
    auto* move_up = new QPushButton{QStringLiteral("Move Up"), this};
    move_up->setObjectName(QStringLiteral("service-order-move-up"));
    auto* move_down = new QPushButton{QStringLiteral("Move Down"), this};
    move_down->setObjectName(QStringLiteral("service-order-move-down"));
    QObject::connect(move_up, &QPushButton::clicked, this, [this] {
        const auto row = list_->currentRow();
        if (row > 0 && row < static_cast<int>(items_.size()) && move_handler_) {
            move_handler_(items_[static_cast<std::size_t>(row)].item.id, row - 1);
        }
    });
    QObject::connect(move_down, &QPushButton::clicked, this, [this] {
        const auto row = list_->currentRow();
        if (row >= 0 && row + 1 < static_cast<int>(items_.size()) && move_handler_) {
            move_handler_(items_[static_cast<std::size_t>(row)].item.id, row + 1);
        }
    });
    controls->addWidget(move_up);
    controls->addWidget(move_down);
    layout->addLayout(controls);
}

void ServiceOrderWidget::set_items(std::vector<ServiceOrderView> items) {
    items_ = std::move(items);
    list_->clear();
    for (const auto& view : items_) {
        list_->addItem(view.label);
    }
}

void ServiceOrderWidget::select_row(int row) { list_->setCurrentRow(row); }

int ServiceOrderWidget::count() const noexcept { return list_->count(); }

}  // namespace shekinah::ui
