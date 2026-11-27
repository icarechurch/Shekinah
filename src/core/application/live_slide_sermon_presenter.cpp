#include "shekinah/application/live_slide_presenter.hpp"

#include <algorithm>
#include <cstddef>

namespace shekinah::application {

LiveSlidePresentation build_live_sermon_presentation(
    const domain::SermonDeck& sermon, const domain::CueId& cue) {
    LiveSlidePresentation result;
    result.item_title = sermon.title;
    result.total_slides = static_cast<int>(sermon.slides.size());

    const auto slide = std::find_if(sermon.slides.begin(), sermon.slides.end(), [&](const auto& candidate) {
        return candidate.id == cue;
    });
    if (slide == sermon.slides.end()) {
        return result;
    }

    const auto index = static_cast<std::size_t>(std::distance(sermon.slides.begin(), slide));
    result.slide_index = static_cast<int>(index + 1);
    result.section = "Slide " + std::to_string(result.slide_index);
    result.background_asset = slide->image;
    if (index + 1 < sermon.slides.size()) {
        result.next_section = "Slide " + std::to_string(result.slide_index + 1);
        result.next_summary = "Next sermon slide";
    }
    return result;
}

}  // namespace shekinah::application
