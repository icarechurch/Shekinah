#include "shekinah/media/audience_output.hpp"

#include "audience_video_output.hpp"

#include <QGuiApplication>
#include <QScreen>

#include <utility>

namespace shekinah::media {

AudienceOutput::AudienceOutput(QWidget* parent) : QWidget(parent) {
    setWindowTitle(QStringLiteral("Shekinah Audience Output"));
    setAttribute(Qt::WA_OpaquePaintEvent);
    resize(1280, 720);
    video_output_ = std::make_unique<AudienceVideoOutput>(this);
    video_output_->set_geometry(rect());
    update_slide_renderer();
}

AudienceOutput::~AudienceOutput() = default;

bool AudienceOutput::video_paused() const noexcept {
    return video_output_->paused();
}

bool AudienceOutput::loop_video_loaded() const noexcept {
    return video_output_->loaded();
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
        video_output_->set_visible(false);
        live_text_.clear();
        slide_background_path_.clear();
        update_slide_renderer();
    } else if (video_output_->loaded()) {
        video_output_->set_visible(true);
    }
    update();
}

void AudienceOutput::set_live_text(QString text) {
    live_text_ = std::move(text);
    update_slide_renderer();
    update();
}

void AudienceOutput::set_video_paused(bool paused) noexcept {
    video_output_->set_paused(paused);
}

}  // namespace shekinah::media
