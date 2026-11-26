#include "shekinah/application/presentation_service.hpp"

#include "shekinah/application/song_backgrounds.hpp"

#include <algorithm>

namespace shekinah::application {

domain::Result<std::optional<domain::BackgroundRef>> PresentationService::resolve_background(
    const domain::ContentRef& content, const domain::CueId& cue) const {
    if (content.kind != domain::ContentKind::song) {
        const auto* sermon = find_sermon(std::get<domain::SermonId>(content.id));
        if (sermon == nullptr) {
            return domain::Result<std::optional<domain::BackgroundRef>>::fail(
                {domain::ErrorCode::not_found, "Sermon was not found"});
        }
        const auto exists = std::any_of(sermon->slides.begin(), sermon->slides.end(), [&](const auto& slide) {
            return slide.id == cue;
        });
        if (!exists) {
            return domain::Result<std::optional<domain::BackgroundRef>>::fail(
                {domain::ErrorCode::not_found, "Sermon cue was not found"});
        }
        return domain::Result<std::optional<domain::BackgroundRef>>::ok(std::nullopt);
    }

    const auto* song = find_song(std::get<domain::SongId>(content.id));
    if (song == nullptr) {
        return domain::Result<std::optional<domain::BackgroundRef>>::fail(
            {domain::ErrorCode::not_found, "Song was not found"});
    }
    return resolve_song_background(*song, cue);
}

}  // namespace shekinah::application
