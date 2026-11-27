#include "shekinah/ui/main_window.hpp"

#include "main_window_live_media.hpp"

#include "shekinah/media/audience_output.hpp"
#include "shekinah/ui/operator_page.hpp"
#include "shekinah/ui/program_status_widget.hpp"

namespace shekinah::ui {
using namespace main_window_internal;

void ShekinahWindow::present_live_media() {
    if (!songs() || !sermons() || !assets()) {
        operator_page_->program_status()->set_slide_preview({});
        operator_page_->program_status()->set_live_details({});
        return;
    }
    const auto snapshot = resolve_live_media(
        *songs(), *sermons(), *assets(), presentation_.program());
    if (!snapshot.has_value()) {
        operator_page_->program_status()->set_slide_preview({});
        operator_page_->program_status()->set_live_details({});
        if (audience_output_) {
            audience_output_->set_live_text({});
            audience_output_->clear_slide_image();
            audience_output_->clear_loop_video();
        }
        return;
    }

    const auto& presented = snapshot->presentation;
    present_live_media_output(
        *snapshot,
        QString::fromStdString(presented.slide_text),
        make_live_details(presented, snapshot->asset));
}

}  // namespace shekinah::ui
