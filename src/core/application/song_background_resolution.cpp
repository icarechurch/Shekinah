#include "shekinah/application/song_backgrounds.hpp"

#include <algorithm>

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

}  // namespace shekinah::application
