#include "live_output_coordinator.hpp"

#include "shekinah/media/audience_output.hpp"
#include "shekinah/ui/operator_page.hpp"
#include "shekinah/ui/program_status_widget.hpp"

#include <utility>

namespace shekinah::ui::main_window_internal {

LiveOutputCoordinator::LiveOutputCoordinator(OperatorPage& operator_page) noexcept
    : operator_page_(operator_page) {}

void LiveOutputCoordinator::set_audience_output(media::AudienceOutput* audience_output) noexcept {
    audience_output_ = audience_output;
}

void LiveOutputCoordinator::clear_preview() const {
    operator_page_.program_status()->set_slide_preview({});
    operator_page_.program_status()->set_live_details({});
}

void LiveOutputCoordinator::clear_output() const {
    clear_preview();
    if (audience_output_ != nullptr) {
        audience_output_->set_live_text({});
        audience_output_->clear_slide_image();
        audience_output_->clear_loop_video();
    }
}

void LiveOutputCoordinator::present(const LiveMediaSnapshot& snapshot,
                                    QString preview_text,
                                    LiveSlideDetails details,
                                    bool media_admitted) const {
    const auto& presented = snapshot.presentation;
    if (audience_output_ != nullptr) {
        audience_output_->set_live_text(preview_text);
    }

    if (!presented.background_asset.has_value()) {
        operator_page_.program_status()->set_slide_preview(std::move(preview_text));
        operator_page_.program_status()->set_live_details(std::move(details));
        if (audience_output_ != nullptr) {
            audience_output_->clear_slide_image();
            audience_output_->clear_loop_video();
        }
        return;
    }
    if (!snapshot.asset.has_value()) {
        operator_page_.program_status()->set_slide_preview(std::move(preview_text));
        operator_page_.program_status()->set_live_details(std::move(details));
        return;
    }

    const auto& asset = *snapshot.asset;
    const auto& path = snapshot.asset_path;
    if (asset.kind == domain::AssetKind::image) {
        operator_page_.program_status()->set_slide_preview(
            std::move(preview_text), QString::fromStdString(path.string()), presented.background_fit);
        operator_page_.program_status()->set_live_details(std::move(details));
        if (audience_output_ != nullptr) {
            audience_output_->clear_loop_video();
            audience_output_->load_slide_image(path, presented.background_fit);
        }
    } else {
        operator_page_.program_status()->set_slide_preview(std::move(preview_text));
        operator_page_.program_status()->set_live_details(std::move(details));
        if (audience_output_ == nullptr || !media_admitted) {
            return;
        }
        audience_output_->clear_slide_image();
        audience_output_->load_loop_video(path);
    }
}

}  // namespace shekinah::ui::main_window_internal
