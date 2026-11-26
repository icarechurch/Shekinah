#include "shekinah/application/song_backgrounds.hpp"

#include <algorithm>
#include <utility>

namespace shekinah::application {

domain::Result<std::optional<domain::BackgroundRef>> resolve_song_background(
    const domain::Song& song, const domain::CueId& cue) {
    const auto slide = std::find_if(song.slides.begin(), song.slides.end(), [&](const auto& candidate) {
        return candidate.id == cue;
    });
    if (slide == song.slides.end()) {
        return domain::Result<std::optional<domain::BackgroundRef>>::fail(
            {domain::ErrorCode::not_found, "Song cue was not found"});
    }
    return domain::Result<std::optional<domain::BackgroundRef>>::ok(
        slide->background.has_value() ? slide->background : song.default_background);
}

domain::Status apply_background_from_slide(domain::Song& song,
                                           const domain::CueId& start,
                                           domain::BackgroundRef background) {
    const auto first = std::find_if(song.slides.begin(), song.slides.end(), [&](const auto& slide) {
        return slide.id == start;
    });
    if (first == song.slides.end()) {
        return domain::Status::fail({domain::ErrorCode::not_found, "Song cue was not found"});
    }
    for (auto slide = first; slide != song.slides.end(); ++slide) {
        slide->background = background;
    }
    return domain::success();
}

void set_default_background(domain::Song& song, domain::BackgroundRef background) {
    song.default_background = std::move(background);
}

void remove_asset_from_song(domain::Song& song, const domain::AssetId& asset) {
    if (song.default_background.has_value() && song.default_background->asset == asset) {
        song.default_background.reset();
    }
    for (auto& slide : song.slides) {
        if (slide.background.has_value() && slide.background->asset == asset) {
            slide.background.reset();
        }
    }
}

}  // namespace shekinah::application
