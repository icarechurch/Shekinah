#include "shekinah/ui/service_order_widget.hpp"

#include <QHBoxLayout>
#include <QLineEdit>
#include <QVBoxLayout>

namespace shekinah::ui {

void ServiceOrderWidget::setup_filter(QVBoxLayout* layout) {
    auto* filter_bar = new QWidget{this};
    filter_bar->setObjectName(QStringLiteral("service-order-filter-bar"));
    filter_bar->setFixedHeight(32);
    auto* filter_layout = new QHBoxLayout(filter_bar);
    filter_layout->setContentsMargins(6, 4, 6, 4);
    filter_layout->setSpacing(0);

    filter_input_ = new QLineEdit{filter_bar};
    filter_input_->setObjectName(QStringLiteral("service-order-filter"));
    filter_input_->setPlaceholderText(QStringLiteral("Filter service..."));
    filter_input_->setClearButtonEnabled(true);
    QObject::connect(filter_input_, &QLineEdit::textChanged, this, [this] { apply_filter(); });
    filter_layout->addWidget(filter_input_);
    layout->addWidget(filter_bar);
}

}  // namespace shekinah::ui
