#include "shekinah/application/presentation_service.hpp"

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

namespace shekinah::application {
namespace {

domain::Status not_found(std::string message) {
    return domain::Status::fail({domain::ErrorCode::not_found, std::move(message)});
}

bool contains_cue(const auto& slides, const domain::CueId& cue) {
    return std::any_of(slides.begin(), slides.end(), [&](const auto& slide) {
        return slide.id == cue;
    });
}

}  // namespace

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
        const auto* song = find_song(std::get<domain::SongId>(item->content.id));
        if (song == nullptr || !contains_cue(song->slides, cue)) {
            return not_found("Song cue was not found");
        }
    } else {
        const auto* sermon = find_sermon(std::get<domain::SermonId>(item->content.id));
        if (sermon == nullptr || !contains_cue(sermon->slides, cue)) {
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

domain::Status PresentationService::move_live(int direction) {
    if (!program_.live_cue.has_value()) {
        return domain::Status::fail({domain::ErrorCode::invalid_input, "There is no live cue to navigate"});
    }

    const auto& live = *program_.live_cue;
    const auto selected = selected_item_.has_value() && selected_item_->content == live.content
                              ? &*selected_item_
                              : find_item_for_content(live.content);
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

const domain::ServiceOrderItem* PresentationService::find_item_for_content(
    const domain::ContentRef& content) const noexcept {
    const auto item = std::find_if(service_order_.begin(), service_order_.end(), [&](const auto& candidate) {
        return candidate.content == content;
    });
    return item == service_order_.end() ? nullptr : &*item;
}

}  // namespace shekinah::application
