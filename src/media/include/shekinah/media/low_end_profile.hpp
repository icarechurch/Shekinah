#pragma once

#include "shekinah/domain/result.hpp"

namespace shekinah::media {

struct VideoInfo {
    int width{0};
    int height{0};
    int frames_per_second{0};
    bool hardware_accelerated{false};
};

class LowEndCompatibilityProfile {
public:
    domain::Status preflight(const VideoInfo& video, int active_video_count = 0) const;
};

}  // namespace shekinah::media
