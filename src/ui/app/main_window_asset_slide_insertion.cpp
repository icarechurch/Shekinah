#include "shekinah/ui/main_window.hpp"

#include "shekinah/storage/content_repository.hpp"
#include "shekinah/ui/operator_page.hpp"
#include "shekinah/ui/service_order_widget.hpp"

namespace shekinah::ui {

void ShekinahWindow::insert_asset_at_slide_position(std::size_t index, domain::AssetId asset) {
    if (!repository()) {
        return;
    }
    const auto selected = presentation_.selected_item();
    if (!selected.has_value()) {
        return;
    }
    const auto asset_record = repository()->load_asset(asset);
    if (!asset_record) {
        return;
    }

    const auto inserted_count = selected->content.kind == domain::ContentKind::song
                                    ? insert_asset_into_song(index, selected->content, *asset_record)
                                    : insert_asset_into_sermon(index, selected->content, *asset_record);
    if (!inserted_count.has_value()) {
        return;
    }
    operator_page_->service_order_widget()->update_item_slide_count(
        selected->content, *inserted_count);
    refresh_cues();
    update_program_status();
}

}  // namespace shekinah::ui
