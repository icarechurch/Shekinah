#include "shekinah/application/cue_presenter.hpp"

#include <optional>
#include <utility>

namespace shekinah::application {
namespace {

std::optional<std::filesystem::path> resolve_asset(
    const AssetPathResolver& resolver, const domain::AssetId& asset) {
    if (!resolver) {
        return std::nullopt;
    }
    return resolver(asset);
}

}  // namespace

CuePresentationSet build_sermon_cue_presentation(
    const domain::SermonDeck& sermon, const AssetPathResolver& resolve_asset_path) {
    CuePresentationSet result;
    result.title = sermon.title;
    result.cues.reserve(sermon.slides.size());
    result.quick_cues.reserve(sermon.slides.size());

    for (std::size_t index = 0; index != sermon.slides.size(); ++index) {
        const auto& slide = sermon.slides[index];
        const auto number = static_cast<int>(index + 1);
        const auto label = "Slide " + std::to_string(number);
        std::filesystem::path background_path;
        if (const auto resolved = resolve_asset(resolve_asset_path, slide.image);
            resolved.has_value()) {
            background_path = *resolved;
        }

        result.quick_cues.push_back(QuickCuePresentation{slide.id, number, label, {}});
        result.cues.push_back(CuePresentation{
            slide.id,
            {},
            {},
            number,
            std::move(background_path),
            domain::FitMode::cover,
            label,
            false,
        });
    }
    return result;
}

}  // namespace shekinah::application
