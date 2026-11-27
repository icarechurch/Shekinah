#include "shekinah/ui/operator_page.hpp"

#include "shekinah/ui/service_order_widget.hpp"

namespace shekinah::ui {

void OperatorPage::set_service_order(std::vector<ServiceOrderView> items) {
    service_order_->set_items(std::move(items));
}

void OperatorPage::set_service_order_selection_handler(
    std::function<void(domain::ServiceOrderItemId)> handler) {
    service_order_->set_selection_handler(std::move(handler));
}

void OperatorPage::set_service_order_move_handler(
    std::function<void(domain::ServiceOrderItemId, std::int64_t)> handler) {
    service_order_->set_move_handler(std::move(handler));
}

void OperatorPage::set_service_order_asset_drop_handler(
    std::function<void(domain::ContentRef, domain::AssetId)> handler) {
    service_order_->set_asset_drop_handler(std::move(handler));
}

void OperatorPage::set_service_order_content_drop_handler(
    std::function<void(domain::ContentRef)> handler) {
    service_order_->set_content_drop_handler(std::move(handler));
}

void OperatorPage::set_service_order_remove_handler(
    std::function<void(domain::ServiceOrderItemId)> handler) {
    service_order_->set_remove_handler(std::move(handler));
}

void OperatorPage::set_service_order_edit_handler(
    std::function<void(domain::ContentRef)> handler) {
    service_order_->set_edit_handler(std::move(handler));
}

void OperatorPage::set_service_order_new_sermon_handler(std::function<void()> handler) {
    new_sermon_handler_ = std::move(handler);
    service_order_->set_authoring_handlers(
        new_song_handler_, import_song_handler_, new_sermon_handler_, import_sermon_handler_);
}

void OperatorPage::set_service_order_live_content(std::optional<domain::ContentRef> live_content) {
    service_order_->set_live_content(std::move(live_content));
}

}  // namespace shekinah::ui
