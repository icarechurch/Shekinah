#include "audience_video_output.hpp"

#include <QMediaPlayer>
#include <QUrl>
#include <QVideoWidget>

namespace shekinah::media {

AudienceVideoOutput::AudienceVideoOutput(QWidget* parent) {
    player_ = new QMediaPlayer{parent};
    player_->setLoops(QMediaPlayer::Infinite);
    widget_ = new QVideoWidget{parent};
    widget_->setVisible(false);
    player_->setVideoOutput(widget_);
}

void AudienceVideoOutput::set_geometry(const QRect& geometry) {
    if (widget_ != nullptr) {
        widget_->setGeometry(geometry);
    }
}

void AudienceVideoOutput::set_visible(bool visible) {
    if (widget_ != nullptr) {
        widget_->setVisible(visible);
    }
}

domain::Status AudienceVideoOutput::load(const std::filesystem::path& path, bool visible) {
    if (!std::filesystem::is_regular_file(path)) {
        return domain::Status::fail(
            {domain::ErrorCode::io_failure, "loop-video path is not a file"});
    }
    player_->stop();
    player_->setSource(QUrl::fromLocalFile(QString::fromStdString(path.string())));
    loaded_ = true;
    paused_ = false;
    set_visible(visible);
    player_->play();
    return domain::success();
}

void AudienceVideoOutput::clear() noexcept {
    player_->stop();
    player_->setSource(QUrl{});
    set_visible(false);
    loaded_ = false;
    paused_ = false;
}

void AudienceVideoOutput::set_paused(bool paused) noexcept {
    paused_ = paused;
    if (player_ == nullptr || !loaded_) {
        return;
    }
    if (paused_) {
        player_->pause();
    } else {
        player_->play();
    }
}

}  // namespace shekinah::media
