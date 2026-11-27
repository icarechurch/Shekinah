#include "shekinah/ui/main_window.hpp"

#include "shekinah/application/slide_insertion.hpp"
#include "content_persistence_coordinator.hpp"
#include "shekinah/storage/sermon_repository.hpp"

namespace shekinah::ui {

std::optional<int> ShekinahWindow::insert_asset_into_sermon(
    std::size_t index,
    const domain::ContentRef& selected_content,
    const domain::AssetRecord& asset) {
    auto sermon = sermons()->load_deck(std::get<domain::SermonId>(selected_content.id));
    if (!sermon) {
        return std::nullopt;
    }
    const auto inserted = application::insert_asset_slide(*sermon, index, asset.id);
    if (!inserted) {
        return std::nullopt;
    }
    const auto saved = content_persistence_coordinator_->save_sermon(*sermon);
    if (!saved) {
        return std::nullopt;
    }
    return static_cast<int>(saved->slides.size());
}

}  // namespace shekinah::ui
