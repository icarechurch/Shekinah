#include "shekinah/application/cue_presenter.hpp"

#include "application_text.hpp"
#include "cue_presenter_asset_resolution.hpp"

#include <optional>
#include <utility>

namespace shekinah::application {

CuePresentationSet build_song_cue_presentation(
    const domain::Song& song, const AssetPathResolver& resolve_asset_path) {
    CuePresentationSet result;
    result.title = song.title;
    result.cues.reserve(song.slides.size());
    result.quick_cues.reserve(song.slides.size());

    std::optional<domain::BackgroundRef> previous_background;
    for (std::size_t index = 0; index != song.slides.size(); ++index) {
        const auto& slide = song.slides[index];
        const auto summary = internal::join_lines(slide.lines);
        const auto background = slide.background.has_value()
                                  ? slide.background
                                  : song.default_background;
        const auto starts_asset_range = background.has_value() &&
                                        (slide.lines.empty() ||
                                         !previous_background.has_value() ||
                                         previous_background != background);

        std::filesystem::path background_path;
        auto background_fit = domain::FitMode::cover;
        if (starts_asset_range) {
            const auto resolved = internal::resolve_asset_path(resolve_asset_path, background->asset);
            if (resolved.has_value()) {
                background_path = *resolved;
                background_fit = background->fit;
            }
        }
        previous_background = background;

        const auto number = static_cast<int>(index + 1);
        result.quick_cues.push_back(
            QuickCuePresentation{slide.id, number, slide.section, summary});
        result.cues.push_back(CuePresentation{
            slide.id,
            slide.section,
            summary,
            number,
            std::move(background_path),
            background_fit,
            slide.section,
            true,
        });
    }
    return result;
}

}  // namespace shekinah::application
