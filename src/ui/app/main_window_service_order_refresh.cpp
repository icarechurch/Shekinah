#include "shekinah/ui/main_window.hpp"

#include "service_order_coordinator.hpp"
#include "service_order_view_coordinator.hpp"
#include "shekinah/ui/cue_grid_widget.hpp"
#include "shekinah/ui/operator_page.hpp"
#include "shekinah/ui/program_status_widget.hpp"
#include "shekinah/ui/service_order_widget.hpp"

#include <utility>

namespace shekinah::ui {

void ShekinahWindow::refresh_service_order() {
    presentation_.reset();
    if (!songs() || !sermons()) {
        operator_page_->set_service_order({});
        operator_page_->cue_grid()->set_cues({});
        operator_page_->set_active_presentation_title({});
        update_program_status();
        return;
    }

    const auto loaded = service_order_coordinator_->load_default_order();
    if (!loaded) {
        operator_page_->set_service_order({});
        operator_page_->cue_grid()->set_cues({});
        operator_page_->set_active_presentation_title({});
        update_program_status();
        return;
    }

    const auto program = presentation_.program();
    auto views = service_order_view_coordinator_->make_views(*loaded, program);
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
