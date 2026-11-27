#include "shekinah/ui/main_window.hpp"

#include "main_window_service_order_views.hpp"
#include "shekinah/storage/content_repository.hpp"
#include "shekinah/ui/cue_grid_widget.hpp"
#include "shekinah/ui/operator_page.hpp"
#include "shekinah/ui/program_status_widget.hpp"
#include "shekinah/ui/service_order_widget.hpp"

#include <utility>

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

    const auto program = presentation_.program();
    auto views = main_window_internal::make_service_order_views(
        *repository(), *repository(), presentation_, *loaded, program);
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
