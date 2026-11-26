#pragma once

#include "shekinah/domain/model.hpp"

#include <optional>
#include <string>

namespace shekinah::application {

struct LiveSlidePresentation {
    std::string item_title;
    std::string section;
    std::string slide_text;
    std::optional<domain::AssetId> background_asset;
    domain::FitMode background_fit{domain::FitMode::cover};
    std::string next_section;
    std::string next_summary;
    int slide_index{0};
    int total_slides{0};
};

[[nodiscard]] LiveSlidePresentation build_live_song_presentation(
    const domain::Song& song, const domain::CueId& cue);
[[nodiscard]] LiveSlidePresentation build_live_sermon_presentation(
    const domain::SermonDeck& sermon, const domain::CueId& cue);

}  // namespace shekinah::application
