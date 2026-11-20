#include "shekinah/media/low_end_profile.hpp"

namespace shekinah::media {

domain::Status LowEndCompatibilityProfile::preflight(const VideoInfo& video, int active_video_count) const {
    if (video.width <= 0 || video.height <= 0 || video.frames_per_second <= 0) {
        return domain::Status::fail(
            {domain::ErrorCode::invalid_input, "Video dimensions and frame rate must be positive"});
    }
    if (active_video_count >= 1) {
        return domain::Status::fail(
            {domain::ErrorCode::unsupported_media, "The N4000 profile permits one active video only"});
    }
    if (!video.hardware_accelerated) {
        return domain::Status::fail(
            {domain::ErrorCode::unsupported_media, "Video hardware acceleration is required on the N4000 profile"});
    }
    if (video.width > 1920 || video.height > 1080 || video.frames_per_second > 30) {
        return domain::Status::fail(
            {domain::ErrorCode::unsupported_media, "The N4000 profile is limited to accelerated 1080p30 video"});
    }
    return domain::success();
}

}  // namespace shekinah::media
