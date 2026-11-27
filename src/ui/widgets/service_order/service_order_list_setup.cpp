#include "shekinah/ui/service_order_widget.hpp"

#include "service_order_list.hpp"

#include <QAbstractItemView>
#include <QFrame>
#include <QListWidget>
#include <QVBoxLayout>

namespace shekinah::ui {

void ServiceOrderWidget::setup_list(QVBoxLayout* layout) {
    auto* list_widget = new service_order_internal::ServiceOrderListWidget{this};
    list_ = list_widget;
    list_->setObjectName(QStringLiteral("service-order-list"));
    list_->setFrameShape(QFrame::NoFrame);
    list_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    list_->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    list_->setSelectionMode(QAbstractItemView::SingleSelection);
    list_->setUniformItemSizes(true);
    list_->setAcceptDrops(true);
    list_->viewport()->setAcceptDrops(true);
    list_->setContextMenuPolicy(Qt::CustomContextMenu);

    build_empty_overlay();
    connect_list_interactions();
    layout->addWidget(list_, 1);
}

}  // namespace shekinah::ui
