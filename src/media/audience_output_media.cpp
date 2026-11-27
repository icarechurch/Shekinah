#include "shekinah/media/audience_output.hpp"

#include "audience_video_output.hpp"

#include <QImageReader>

namespace shekinah::media {

domain::Status AudienceOutput::load_slide_image(
    const std::filesystem::path& path, domain::FitMode background_fit) {
    if (!std::filesystem::is_regular_file(path)) {
        return domain::Status::fail({domain::ErrorCode::io_failure, "slide image path is not a file"});
    }
    QImageReader reader{QString::fromStdString(path.string())};
    reader.setAutoTransform(true);
    const auto source_size = reader.size();
    if (source_size.isValid() && (source_size.width() > 1920 || source_size.height() > 1080)) {
        reader.setScaledSize(source_size.scaled(1920, 1080, Qt::KeepAspectRatio));
    }
    const auto image = reader.read();
    if (image.isNull()) {
        return domain::Status::fail({domain::ErrorCode::io_failure, "unable to decode slide image"});
    }
    slide_image_loaded_ = true;
    slide_background_path_ = QString::fromStdString(path.string());
    slide_background_fit_ = background_fit;
    update_slide_renderer();
    update();
    return domain::success();
}

void AudienceOutput::clear_slide_image() noexcept {
    slide_image_loaded_ = false;
    slide_background_path_.clear();
    update_slide_renderer();
    update();
}

domain::Status AudienceOutput::load_loop_video(const std::filesystem::path& path) {
    if (!std::filesystem::is_regular_file(path)) {
        return domain::Status::fail({domain::ErrorCode::io_failure, "loop-video path is not a file"});
    }
    return video_output_->load(path, program_.mode == domain::ProgramMode::cue);
}

void AudienceOutput::clear_loop_video() {
    video_output_->clear();
}

}  // namespace shekinah::media
