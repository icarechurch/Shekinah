#include "shekinah/ui/service_order_widget.hpp"

#include <QListWidget>
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
