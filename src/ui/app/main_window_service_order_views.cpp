#include "main_window_service_order_views.hpp"

#include "shekinah/application/service_order_presenter.hpp"

#include <QString>

namespace shekinah::ui::main_window_internal {

std::vector<ServiceOrderView> make_service_order_views(
    const storage::SongRepository& songs,
    const storage::SermonRepository& sermons,
    application::PresentationService& presentation,
    const std::vector<domain::ServiceOrderItem>& items,
    const domain::ProgramState& program) {
    std::vector<ServiceOrderView> views;
    views.reserve(items.size());
    for (const auto& item : items) {
        application::ServiceOrderItemPresentation presented;
        if (item.content.kind == domain::ContentKind::song) {
            const auto content_id = std::get<domain::SongId>(item.content.id);
            const auto song = songs.load_song(content_id);
            if (song) {
                presentation.register_song(*song);
                presented = application::present_service_order_item(
                    item, &*song, nullptr, program);
            } else {
                presented = application::present_service_order_item(
                    item, nullptr, nullptr, program);
            }
        } else {
            const auto content_id = std::get<domain::SermonId>(item.content.id);
            const auto sermon = sermons.load_deck(content_id);
            if (sermon) {
                presentation.register_sermon(*sermon);
                presented = application::present_service_order_item(
                    item, nullptr, &*sermon, program);
            } else {
                presented = application::present_service_order_item(
                    item, nullptr, nullptr, program);
            }
        }
        views.push_back(ServiceOrderView{
            item,
            QString::fromStdString(presented.label),
            presented.slide_count,
            presented.has_background,
            presented.is_live,
        });
    }
    return views;
}

}  // namespace shekinah::ui::main_window_internal
