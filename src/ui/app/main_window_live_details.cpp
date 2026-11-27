#include "main_window_live_media.hpp"

#include <QString>

namespace shekinah::ui::main_window_internal {

LiveSlideDetails make_live_details(
    const application::LiveSlidePresentation& presentation,
    const std::optional<domain::AssetRecord>& asset) {
    LiveSlideDetails details{};
    details.item_title = QString::fromStdString(presentation.item_title);
    details.section = QString::fromStdString(presentation.section);
    details.slide_text = QString::fromStdString(presentation.slide_text);
    details.next_section = QString::fromStdString(presentation.next_section);
    details.next_summary = QString::fromStdString(presentation.next_summary);
    details.slide_index = presentation.slide_index;
    details.total_slides = presentation.total_slides;
    if (presentation.background_asset.has_value()) {
        details.fit_mode = presentation.background_fit;
    }
    if (asset.has_value()) {
        const auto filename = std::filesystem::path{asset->relative_path}.filename().string();
        details.has_background = true;
        details.is_video = (asset->kind == domain::AssetKind::video);
        details.asset_name = QString::fromStdString(filename.empty() ? asset->relative_path : filename);
    }
    return details;
}

}  // namespace shekinah::ui::main_window_internal
