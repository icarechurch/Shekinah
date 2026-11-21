#include "shekinah/application/song_backgrounds.hpp"

#include <algorithm>
#include <utility>

namespace shekinah::application {

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

}  // namespace shekinah::application
