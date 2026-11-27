#include "shekinah/ui/main_window.hpp"

#include "shekinah/application/slide_insertion.hpp"
#include "shekinah/storage/song_repository.hpp"
#include "shekinah/ui/operator_page.hpp"

namespace shekinah::ui {

std::optional<int> ShekinahWindow::insert_asset_into_song(
    std::size_t index,
    const domain::ContentRef& selected_content,
    const domain::AssetRecord& asset) {
    auto song = songs()->load_song(std::get<domain::SongId>(selected_content.id));
    if (!song) {
        return std::nullopt;
    }
    const auto inserted = application::insert_asset_slide(
        *song, index, domain::BackgroundRef{asset.id, domain::FitMode::cover});
    if (!inserted) {
        return std::nullopt;
    }
    const auto saved = songs()->save_song(*song);
    if (!saved) {
        return std::nullopt;
    }
    presentation_.register_song(*saved);
    return static_cast<int>(saved->slides.size());
}

}  // namespace shekinah::ui
