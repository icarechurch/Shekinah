#include "shekinah/application/service_order_presenter.hpp"

#include <string>

namespace shekinah::application {

ServiceOrderItemPresentation present_service_order_item(
    const domain::ServiceOrderItem& item,
    const domain::Song* song,
    const domain::SermonDeck* sermon,
    const domain::ProgramState& program) {
    ServiceOrderItemPresentation result;
    if (item.content.kind == domain::ContentKind::song) {
        const auto& content_id = std::get<domain::SongId>(item.content.id);
        if (song != nullptr) {
            result.label = song->title;
            result.slide_count = static_cast<int>(song->slides.size());
            result.has_background = song->default_background.has_value();
        } else {
            result.label = "Song " + content_id.value;
        }
    } else {
        const auto& content_id = std::get<domain::SermonId>(item.content.id);
        if (sermon != nullptr) {
            result.label = sermon->title;
            result.slide_count = static_cast<int>(sermon->slides.size());
        } else {
            result.label = "Sermon " + content_id.value;
        }
    }

    result.is_live = program.mode == domain::ProgramMode::cue &&
                     program.live_cue.has_value() &&
                     program.live_cue->content == item.content;
    return result;
}

}  // namespace shekinah::application
