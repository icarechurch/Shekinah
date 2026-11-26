#pragma once

#include "shekinah/domain/model.hpp"

#include <filesystem>
#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace shekinah::application {

struct CuePresentation {
    domain::CueId id;
    std::string section;
    std::string summary;
    int number{0};
    std::filesystem::path background_path;
    domain::FitMode background_fit{domain::FitMode::cover};
    std::string footer_label;
    bool show_text{true};
};

struct QuickCuePresentation {
    domain::CueId id;
    int number{0};
    std::string label;
    std::string summary;
};

struct CuePresentationSet {
    std::string title;
    std::vector<CuePresentation> cues;
    std::vector<QuickCuePresentation> quick_cues;
};

using AssetPathResolver =
    std::function<std::optional<std::filesystem::path>(const domain::AssetId&)>;

[[nodiscard]] CuePresentationSet build_song_cue_presentation(
    const domain::Song& song, const AssetPathResolver& resolve_asset_path = {});
[[nodiscard]] CuePresentationSet build_sermon_cue_presentation(
    const domain::SermonDeck& sermon, const AssetPathResolver& resolve_asset_path = {});

}  // namespace shekinah::application
