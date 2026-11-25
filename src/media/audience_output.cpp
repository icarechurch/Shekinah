#include "shekinah/media/audience_output.hpp"

#include <QGuiApplication>
#include <QImageReader>
#include <QMediaPlayer>
#include <QPainter>
#include <QResizeEvent>
#include <QScreen>
#include <QUrl>
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
    slide_image_ = image;
    slide_image_loaded_ = true;
    slide_background_path_ = QString::fromStdString(path.string());
    slide_background_fit_ = background_fit;
    update_slide_renderer();
    update();
    return domain::success();
}

void AudienceOutput::clear_slide_image() noexcept {
    slide_image_ = QImage{};
    slide_image_loaded_ = false;
    slide_background_path_.clear();
    update_slide_renderer();
    update();
}

domain::Status AudienceOutput::load_loop_video(const std::filesystem::path& path) {
    if (!std::filesystem::is_regular_file(path)) {
        return domain::Status::fail({domain::ErrorCode::io_failure, "loop-video path is not a file"});
    }
    player_->stop();
    player_->setSource(QUrl::fromLocalFile(QString::fromStdString(path.string())));
    loop_video_loaded_ = true;
    video_paused_ = false;
    if (program_.mode == domain::ProgramMode::cue) {
        video_widget_->setVisible(true);
    }
    player_->play();
    return domain::success();
}

void AudienceOutput::clear_loop_video() {
    player_->stop();
    player_->setSource(QUrl{});
    video_widget_->setVisible(false);
    loop_video_loaded_ = false;
    video_paused_ = false;
}

void AudienceOutput::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter painter{this};
    switch (program_.mode) {
    case domain::ProgramMode::black:
        painter.fillRect(rect(), Qt::black);
        return;
    case domain::ProgramMode::cue:
        slide_renderer_.paint(painter, rect());
        return;
    case domain::ProgramMode::clear:
        painter.fillRect(rect(), QColor{QStringLiteral("#101318")});
        return;
    }

    painter.fillRect(rect(), QColor{QStringLiteral("#101318")});
}

void AudienceOutput::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    if (video_widget_ != nullptr) {
        video_widget_->setGeometry(rect());
    }
}

void AudienceOutput::update_slide_renderer() {
    slide_renderer_.set_content(SlideRenderData{
        live_text_,
        slide_background_path_,
        slide_background_fit_,
        true,
        false,
        SlideTextMode::audience,
    });
}

}  // namespace shekinah::media
