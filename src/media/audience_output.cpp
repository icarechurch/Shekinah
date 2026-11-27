#include "shekinah/media/audience_output.hpp"

#include <QGuiApplication>
#include <QMediaPlayer>
#include <QScreen>
#include <QVideoWidget>

#include <utility>

namespace shekinah::media {

AudienceOutput::AudienceOutput(QWidget* parent) : QWidget(parent) {
    setWindowTitle(QStringLiteral("Shekinah Audience Output"));
    setAttribute(Qt::WA_OpaquePaintEvent);
    resize(1280, 720);
    player_ = new QMediaPlayer{this};
    player_->setLoops(QMediaPlayer::Infinite);
    video_widget_ = new QVideoWidget{this};
    video_widget_->setGeometry(rect());
    video_widget_->setVisible(false);
    player_->setVideoOutput(video_widget_);
    update_slide_renderer();
}

void AudienceOutput::show_fullscreen(QScreen* screen) {
    if (screen == nullptr) {
        screen = QGuiApplication::primaryScreen();
    }
    if (screen == nullptr) {
        return;
    }
    setWindowFlag(Qt::Window, true);
    setGeometry(screen->geometry());
    mode_ = OutputMode::fullscreen;
    showFullScreen();
}

void AudienceOutput::show_movable() {
    setWindowFlag(Qt::Window, true);
    mode_ = OutputMode::movable;
    showNormal();
}

void AudienceOutput::present(const domain::ProgramState& program) {
    program_ = program;
    if (program_.mode != domain::ProgramMode::cue) {
        video_widget_->setVisible(false);
        live_text_.clear();
        slide_background_path_.clear();
        update_slide_renderer();
    } else if (loop_video_loaded_) {
        video_widget_->setVisible(true);
    }
    update();
}

void AudienceOutput::set_live_text(QString text) {
    live_text_ = std::move(text);
    update_slide_renderer();
    update();
}

void AudienceOutput::set_video_paused(bool paused) noexcept {
    video_paused_ = paused;
    if (player_ == nullptr || !loop_video_loaded_) {
        return;
    }
    if (video_paused_) {
        player_->pause();
    } else {
        player_->play();
    }
}

}  // namespace shekinah::media
