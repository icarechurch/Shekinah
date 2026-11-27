#include "shekinah/application/presentation_service.hpp"

#include <algorithm>
#include <utility>

namespace shekinah::application {

void PresentationService::register_song(domain::Song song) {
    const auto existing = std::find_if(songs_.begin(), songs_.end(), [&](const auto& candidate) {
        return candidate.id == song.id;
    });
    if (existing == songs_.end()) {
        songs_.push_back(std::move(song));
    } else {
        *existing = std::move(song);
    }
}

void PresentationService::register_sermon(domain::SermonDeck deck) {
    const auto existing = std::find_if(sermons_.begin(), sermons_.end(), [&](const auto& candidate) {
        return candidate.id == deck.id;
    });
    if (existing == sermons_.end()) {
        sermons_.push_back(std::move(deck));
    } else {
        *existing = std::move(deck);
    }
}

const domain::Song* PresentationService::find_song(const domain::SongId& id) const noexcept {
    const auto song = std::find_if(songs_.begin(), songs_.end(), [&](const auto& candidate) {
        return candidate.id == id;
    });
    return song == songs_.end() ? nullptr : &*song;
}

const domain::SermonDeck* PresentationService::find_sermon(const domain::SermonId& id) const noexcept {
    const auto sermon = std::find_if(sermons_.begin(), sermons_.end(), [&](const auto& candidate) {
        return candidate.id == id;
    });
    return sermon == sermons_.end() ? nullptr : &*sermon;
}

}  // namespace shekinah::application
