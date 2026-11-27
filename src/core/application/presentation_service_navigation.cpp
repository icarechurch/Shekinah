#include "shekinah/application/presentation_service.hpp"

#include <algorithm>
#include <string>
#include <utility>

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

void PresentationService::clear() noexcept {
    program_ = domain::ProgramState{domain::ProgramMode::clear, std::nullopt};
}

void PresentationService::black() noexcept {
    program_ = domain::ProgramState{domain::ProgramMode::black, std::nullopt};
}

const domain::ServiceOrderItem* PresentationService::find_item_for_content(
    const domain::ContentRef& content) const noexcept {
    const auto item = std::find_if(service_order_.begin(), service_order_.end(), [&](const auto& candidate) {
        return candidate.content == content;
    });
    return item == service_order_.end() ? nullptr : &*item;
}

}  // namespace shekinah::application
