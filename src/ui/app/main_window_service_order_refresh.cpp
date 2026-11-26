#include "shekinah/ui/main_window.hpp"

#include "shekinah/application/service_order_presenter.hpp"
#include "shekinah/storage/content_repository.hpp"
#include "shekinah/ui/cue_grid_widget.hpp"
#include "shekinah/ui/operator_page.hpp"
#include "shekinah/ui/program_status_widget.hpp"
#include "shekinah/ui/service_order_widget.hpp"

#include <QString>

#include <utility>
#include <vector>

namespace shekinah::ui {

void ShekinahWindow::refresh_service_order() {
    presentation_.reset();
    if (!repository()) {
        operator_page_->set_service_order({});
        operator_page_->cue_grid()->set_cues({});
        operator_page_->set_active_presentation_title({});
        update_program_status();
        return;
    }

    const auto loaded = repository()->load_order(domain::ServiceOrderId{"default-service"});
    if (!loaded) {
        operator_page_->set_service_order({});
        operator_page_->cue_grid()->set_cues({});
        operator_page_->set_active_presentation_title({});
        update_program_status();
        return;
    }

    std::vector<ServiceOrderView> views;
    views.reserve(loaded->size());
    const auto program = presentation_.program();
    for (const auto& item : *loaded) {
        application::ServiceOrderItemPresentation presented;
        if (item.content.kind == domain::ContentKind::song) {
            const auto content_id = std::get<domain::SongId>(item.content.id);
            const auto song = repository()->load_song(content_id);
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
            const auto sermon = repository()->load_deck(content_id);
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
    presentation_.set_service_order(*loaded);
    operator_page_->set_service_order(std::move(views));
    if (!loaded->empty()) {
        operator_page_->service_order_widget()->select_row(0);
    } else {
        operator_page_->cue_grid()->set_cues({});
        operator_page_->set_active_presentation_title({});
    }
    update_program_status();
}

}  // namespace shekinah::ui
