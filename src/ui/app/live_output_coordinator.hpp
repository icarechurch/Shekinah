#pragma once

#include "main_window_live_media.hpp"

#include <QString>

namespace shekinah::media {
class AudienceOutput;
}

namespace shekinah::ui {
class OperatorPage;
}

namespace shekinah::ui::main_window_internal {

class LiveOutputCoordinator final {
public:
    explicit LiveOutputCoordinator(OperatorPage& operator_page) noexcept;

    void set_audience_output(media::AudienceOutput* audience_output) noexcept;
    void clear_preview() const;
    void clear_output() const;
    void present(const LiveMediaSnapshot& snapshot,
                 QString preview_text,
                 LiveSlideDetails details,
                 bool media_admitted) const;

private:
    OperatorPage& operator_page_;
    media::AudienceOutput* audience_output_{nullptr};
};

}  // namespace shekinah::ui::main_window_internal
