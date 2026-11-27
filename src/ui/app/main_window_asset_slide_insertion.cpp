#include "shekinah/ui/main_window.hpp"

#include "asset_slide_insertion_coordinator.hpp"

#include "shekinah/ui/operator_page.hpp"
#include "shekinah/ui/service_order_widget.hpp"

namespace shekinah::ui {

void ShekinahWindow::insert_asset_at_slide_position(std::size_t index, domain::AssetId asset) {
    const auto selected = presentation_.selected_item();
    if (!selected.has_value()) {
        return;
    }
    const auto inserted_count =
        asset_slide_insertion_coordinator_->insert(index, selected->content, asset);
    if (!inserted_count.has_value()) {
        return;
    }
    operator_page_->service_order_widget()->update_item_slide_count(
        selected->content, *inserted_count);
    refresh_cues();
    update_program_status();
}

}  // namespace shekinah::ui
