#include "shekinah/application/presentation_service.hpp"

#include <algorithm>
#include <utility>

namespace shekinah::application {

void PresentationService::reset() noexcept {
    songs_.clear();
    sermons_.clear();
    service_order_.clear();
    selected_item_.reset();
    program_ = domain::ProgramState{};
}

domain::Status PresentationService::set_service_order(std::vector<domain::ServiceOrderItem> items) {
    service_order_ = std::move(items);
    if (selected_item_.has_value()) {
        const auto selected = find_item(selected_item_->id);
        if (selected == nullptr) {
            selected_item_.reset();
        } else {
            selected_item_ = *selected;
        }
    }
    return domain::success();
}

const domain::ServiceOrderItem* PresentationService::find_item(
    const domain::ServiceOrderItemId& item_id) const noexcept {
    const auto item = std::find_if(service_order_.begin(), service_order_.end(), [&](const auto& candidate) {
        return candidate.id == item_id;
    });
    return item == service_order_.end() ? nullptr : &*item;
}

}  // namespace shekinah::application
