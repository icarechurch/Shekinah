#include "shekinah/ui/main_window.hpp"

#include "shekinah/media/audience_output.hpp"
#include "shekinah/ui/support/app_icon.hpp"

namespace shekinah::ui {

media::AudienceOutput* ShekinahWindow::audience_output() {
    if (!audience_output_) {
        audience_output_ = std::make_unique<media::AudienceOutput>();
        audience_output_->setWindowIcon(app_icon());
        audience_output_->present(presentation_.program());
        present_live_media();
    }
    return audience_output_.get();
}

}  // namespace shekinah::ui
