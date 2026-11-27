#include "service_order_view_coordinator.hpp"

#include "shekinah/application/presentation_service.hpp"
#include "shekinah/application/service_order_presenter.hpp"
#include "shekinah/storage/project_session.hpp"
#include "shekinah/storage/sermon_repository.hpp"
#include "shekinah/storage/song_repository.hpp"

#include <QString>

namespace shekinah::ui {

ServiceOrderViewCoordinator::ServiceOrderViewCoordinator(
    storage::ProjectSession& session, application::PresentationService& presentation) noexcept
    : session_(session), presentation_(presentation) {}

std::vector<ServiceOrderView> ServiceOrderViewCoordinator::make_views(
    const std::vector<domain::ServiceOrderItem>& items, const domain::ProgramState& program) {
    std::vector<ServiceOrderView> views;
    views.reserve(items.size());

    const auto* songs = session_.songs();
    const auto* sermons = session_.sermons();
    if (songs == nullptr || sermons == nullptr) {
        return views;
    }

    for (const auto& item : items) {
        application::ServiceOrderItemPresentation presented;
        if (item.content.kind == domain::ContentKind::song) {
            const auto content_id = std::get<domain::SongId>(item.content.id);
            const auto song = songs->load_song(content_id);
            if (song) {
                presentation_.register_song(*song);
                presented = application::present_service_order_item(
                    item, &*song, nullptr, program);
            } else {
                presented = application::present_service_order_item(
                    item, nullptr, nullptr, program);
            }
        } else {
            const auto content_id = std::get<domain::SermonId>(item.content.id);
            const auto sermon = sermons->load_deck(content_id);
            if (sermon) {
                presentation_.register_sermon(*sermon);
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

}  // namespace shekinah::ui
