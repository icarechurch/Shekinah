#include "service_order_list.hpp"

#include "service_order_delegate.hpp"

namespace shekinah::ui::service_order_internal {

ServiceOrderListWidget::ServiceOrderListWidget(QWidget* parent) : QListWidget(parent) {
    viewport()->installEventFilter(this);
    setMouseTracking(true);
    setItemDelegate(new ServiceOrderItemDelegate{this});
}

void ServiceOrderListWidget::set_asset_drop_handler(AssetDropHandler handler) {
    asset_drop_handler_ = std::move(handler);
}

void ServiceOrderListWidget::set_content_drop_handler(ContentDropHandler handler) {
    content_drop_handler_ = std::move(handler);
}

void ServiceOrderListWidget::set_key_action_handlers(std::function<void(int)> move_delta,
                                                     std::function<void()> remove_selected,
                                                     std::function<void()> edit_selected) {
    key_move_delta_ = std::move(move_delta);
    key_remove_ = std::move(remove_selected);
    key_edit_ = std::move(edit_selected);
}

void ServiceOrderListWidget::set_viewport_resized_handler(std::function<void()> handler) {
    viewport_resized_ = std::move(handler);
}

}  // namespace shekinah::ui::service_order_internal
