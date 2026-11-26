#include "shekinah/application/cue_presenter.hpp"

#include <optional>
#include <sstream>
#include <utility>

namespace shekinah::application {
namespace {

std::string join_lines(const std::vector<std::string>& lines) {
    std::ostringstream out;
    for (std::size_t index = 0; index != lines.size(); ++index) {
        if (index != 0) {
            out << '\n';
        }
        out << lines[index];
    }
    return out.str();
}

std::optional<std::filesystem::path> resolve_asset(
    const AssetPathResolver& resolver, const domain::AssetId& asset) {
    if (!resolver) {
        return std::nullopt;
    }
    return resolver(asset);
}

}  // namespace

CuePresentationSet build_song_cue_presentation(
    const domain::Song& song, const AssetPathResolver& resolve_asset_path) {
    CuePresentationSet result;
    result.title = song.title;
    result.cues.reserve(song.slides.size());
    result.quick_cues.reserve(song.slides.size());

    std::optional<domain::BackgroundRef> previous_background;
    for (std::size_t index = 0; index != song.slides.size(); ++index) {
        const auto& slide = song.slides[index];
        const auto summary = join_lines(slide.lines);
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
            const auto resolved = resolve_asset(resolve_asset_path, background->asset);
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
