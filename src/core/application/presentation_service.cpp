#include "shekinah/application/presentation_service.hpp"

#include <algorithm>
#include <string>
#include <utility>

namespace shekinah::application {
namespace {

domain::Status not_found(std::string message) {
    return domain::Status::fail({domain::ErrorCode::not_found, std::move(message)});
}

}  // namespace

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

domain::Status PresentationService::set_service_order(std::vector<domain::ServiceOrderItem> items) {
    service_order_ = std::move(items);
    if (selected_item_.has_value()) {
        const auto selected = find_item(selected_item_->id);
        if (selected == nullptr) {
            selected_item_.reset();
        } else {
            selected_item_ = *selected;
        }
    }
    return domain::success();
}

domain::Status PresentationService::select_service_item(const domain::ServiceOrderItemId& item_id) {
    const auto item = find_item(item_id);
    if (item == nullptr) {
        return not_found("Service-order item was not found");
    }
    selected_item_ = *item;
    return domain::success();
}

domain::Status PresentationService::take_live(
    const domain::ServiceOrderItemId& item_id, const domain::CueId& cue) {
    const auto item = find_item(item_id);
    if (item == nullptr) {
        return not_found("Service-order item was not found");
    }

    if (item->content.kind == domain::ContentKind::song) {
        const auto& song_id = std::get<domain::SongId>(item->content.id);
        const auto* song = find_song(song_id);
        if (song == nullptr || std::none_of(song->slides.begin(), song->slides.end(), [&](const auto& slide) {
                return slide.id == cue;
            })) {
            return not_found("Song cue was not found");
        }
    } else {
        const auto& sermon_id = std::get<domain::SermonId>(item->content.id);
        const auto* sermon = find_sermon(sermon_id);
        if (sermon == nullptr || std::none_of(sermon->slides.begin(), sermon->slides.end(), [&](const auto& slide) {
                return slide.id == cue;
            })) {
            return not_found("Sermon cue was not found");
        }
    }

    selected_item_ = *item;
    program_ = domain::ProgramState{domain::ProgramMode::cue, domain::CueRef{item->content, cue}};
    return domain::success();
}

domain::Status PresentationService::previous() { return move_live(-1); }

domain::Status PresentationService::next() { return move_live(1); }

void PresentationService::clear() noexcept {
    program_ = domain::ProgramState{domain::ProgramMode::clear, std::nullopt};
}

void PresentationService::black() noexcept {
    program_ = domain::ProgramState{domain::ProgramMode::black, std::nullopt};
}

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
    const auto slide = std::find_if(song->slides.begin(), song->slides.end(), [&](const auto& candidate) {
        return candidate.id == cue;
    });
    if (slide == song->slides.end()) {
        return domain::Result<std::optional<domain::BackgroundRef>>::fail(
            {domain::ErrorCode::not_found, "Song cue was not found"});
    }
    if (slide->background.has_value()) {
        return domain::Result<std::optional<domain::BackgroundRef>>::ok(slide->background);
    }
    return domain::Result<std::optional<domain::BackgroundRef>>::ok(song->default_background);
}

const domain::ServiceOrderItem* PresentationService::find_item(
    const domain::ServiceOrderItemId& item_id) const noexcept {
    const auto item = std::find_if(service_order_.begin(), service_order_.end(), [&](const auto& candidate) {
        return candidate.id == item_id;
    });
    return item == service_order_.end() ? nullptr : &*item;
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

domain::Status PresentationService::move_live(int direction) {
    if (!program_.live_cue.has_value()) {
        return domain::Status::fail({domain::ErrorCode::invalid_input, "There is no live cue to navigate"});
    }

    const auto& live = *program_.live_cue;
    const auto item = selected_item_.has_value() && selected_item_->content == live.content
                          ? &*selected_item_
                          : nullptr;
    const auto* selected = item;
    if (selected == nullptr) {
        selected = std::find_if(service_order_.begin(), service_order_.end(), [&](const auto& candidate) {
                     return candidate.content == live.content;
                 }) == service_order_.end()
                       ? nullptr
                       : &*std::find_if(service_order_.begin(), service_order_.end(), [&](const auto& candidate) {
                             return candidate.content == live.content;
                         });
    }
    if (selected == nullptr) {
        return not_found("Live content is not in the service order");
    }

    std::vector<domain::CueId> cues;
    if (live.content.kind == domain::ContentKind::song) {
        const auto* song = find_song(std::get<domain::SongId>(live.content.id));
        if (song != nullptr) {
            for (const auto& slide : song->slides) {
                cues.push_back(slide.id);
            }
        }
    } else {
        const auto* sermon = find_sermon(std::get<domain::SermonId>(live.content.id));
        if (sermon != nullptr) {
            for (const auto& slide : sermon->slides) {
                cues.push_back(slide.id);
            }
        }
    }
    const auto current = std::find(cues.begin(), cues.end(), live.cue);
    if (current == cues.end()) {
        return not_found("Live cue is not in the content deck");
    }
    const auto current_index = static_cast<std::ptrdiff_t>(std::distance(cues.begin(), current));
    const auto target_index = current_index + direction;
    if (target_index < 0 || target_index >= static_cast<std::ptrdiff_t>(cues.size())) {
        return domain::Status::fail({domain::ErrorCode::invalid_input, "Already at the edge of the content"});
    }
    return take_live(selected->id, cues[static_cast<std::size_t>(target_index)]);
}

}  // namespace shekinah::application
