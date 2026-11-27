#pragma once

class QObject;
class QAbstractItemDelegate;

namespace shekinah::ui::service_order_internal {

QAbstractItemDelegate* make_service_order_item_delegate(QObject* parent);

}  // namespace shekinah::ui::service_order_internal
