#include "shekinah/ui/main_window.hpp"

#include "main_window_live_media.hpp"
#include "live_output_coordinator.hpp"

namespace shekinah::ui {
using namespace main_window_internal;

void ShekinahWindow::present_live_media() {
    if (!live_media_coordinator_->ready()) {
        live_output_coordinator_->clear_preview();
        return;
    }
    const auto snapshot = live_media_coordinator_->resolve(presentation_.program());
    if (!snapshot.has_value()) {
        live_output_coordinator_->clear_output();
        return;
    }

    const auto& presented = snapshot->presentation;
    live_output_coordinator_->present(
        *snapshot,
        QString::fromStdString(presented.slide_text),
        make_live_details(presented, snapshot->asset),
        live_media_admitted());
}

}  // namespace shekinah::ui
