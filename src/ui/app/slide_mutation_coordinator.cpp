#include "slide_mutation_coordinator.hpp"

#include "content_persistence_coordinator.hpp"
#include "shekinah/storage/project_session.hpp"
#include "shekinah/storage/sermon_repository.hpp"
#include "shekinah/storage/song_repository.hpp"

#include <algorithm>
#include <string>
#include <utility>
#include <variant>

namespace shekinah::ui {
namespace {

domain::Status mutation_unavailable() {
    return domain::Status::fail(
        {domain::ErrorCode::storage_failure, "Slide mutation is unavailable"});
}

template <typename Deck, typename Save>
domain::Status erase_from_deck(Deck& deck, const domain::CueId& cue, Save&& save) {
    if (deck.slides.size() <= 1) {
        return mutation_unavailable();
    }
    const auto position = std::find_if(deck.slides.begin(), deck.slides.end(), [&](const auto& slide) {
        return slide.id == cue;
    });
    if (position == deck.slides.end()) {
        return mutation_unavailable();
    }
    deck.slides.erase(position);
    const auto saved = save(deck);
    return saved ? domain::success() : domain::Status::fail(saved.error());
}

template <typename Deck, typename Save>
domain::Status duplicate_in_deck(Deck& deck, const domain::CueId& cue, Save&& save) {
    const auto position = std::find_if(deck.slides.begin(), deck.slides.end(), [&](const auto& slide) {
        return slide.id == cue;
    });
    if (position == deck.slides.end()) {
        return mutation_unavailable();
    }
    auto duplicate = *position;
    duplicate.id = domain::CueId{deck.id.value + "-copy-" + std::to_string(deck.slides.size())};
    deck.slides.insert(position + 1, std::move(duplicate));
    const auto saved = save(deck);
    return saved ? domain::success() : domain::Status::fail(saved.error());
}

}  // namespace

SlideMutationCoordinator::SlideMutationCoordinator(
    storage::ProjectSession& session, ContentPersistenceCoordinator& persistence) noexcept
    : session_(session), persistence_(persistence) {}

domain::Status SlideMutationCoordinator::erase(
    const domain::ContentRef& content, const domain::CueId& cue) const {
    if (!session_.songs() || !session_.sermons()) {
        return mutation_unavailable();
    }
    if (content.kind == domain::ContentKind::song) {
        auto song = session_.songs()->load_song(std::get<domain::SongId>(content.id));
        if (!song) {
            return mutation_unavailable();
        }
        return erase_from_deck(*song, cue,
                               [&](const auto& value) { return persistence_.save_song(value); });
    }

    auto sermon = session_.sermons()->load_deck(std::get<domain::SermonId>(content.id));
    if (!sermon) {
        return mutation_unavailable();
    }
    return erase_from_deck(*sermon, cue,
                           [&](const auto& value) { return persistence_.save_sermon(value); });
}

domain::Status SlideMutationCoordinator::duplicate(
    const domain::ContentRef& content, const domain::CueId& cue) const {
    if (!session_.songs() || !session_.sermons()) {
        return mutation_unavailable();
    }
    if (content.kind == domain::ContentKind::song) {
        auto song = session_.songs()->load_song(std::get<domain::SongId>(content.id));
        if (!song) {
            return mutation_unavailable();
        }
        return duplicate_in_deck(
            *song, cue, [&](const auto& value) { return persistence_.save_song(value); });
    }

    auto sermon = session_.sermons()->load_deck(std::get<domain::SermonId>(content.id));
    if (!sermon) {
        return mutation_unavailable();
    }
    return duplicate_in_deck(
        *sermon, cue, [&](const auto& value) { return persistence_.save_sermon(value); });
}

}  // namespace shekinah::ui
