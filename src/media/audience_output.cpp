#include "shekinah/media/audience_output.hpp"

#include <QGuiApplication>
#include <QMediaPlayer>
#include <QPainter>
#include <QResizeEvent>
#include <QScreen>
#include <QUrl>
#include <QVideoWidget>

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
    } else if (loop_video_loaded_) {
        video_widget_->setVisible(true);
    }
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
    if (program_.mode == domain::ProgramMode::black) {
        painter.fillRect(rect(), Qt::black);
        return;
    }

    const QColor background = program_.mode == domain::ProgramMode::clear
                                  ? QColor{QStringLiteral("#101318")}
                                  : QColor{QStringLiteral("#182337")};
    painter.fillRect(rect(), background);
    painter.setPen(Qt::white);
    painter.setFont(QFont{QStringLiteral("Segoe UI"), 28});
    const QString text = program_.mode == domain::ProgramMode::clear
                             ? QStringLiteral("CLEAR")
                             : program_.live_cue.has_value()
                                   ? QString::fromStdString(program_.live_cue->cue.value)
                                   : QStringLiteral("SHEKINAH");
    painter.drawText(rect(), Qt::AlignCenter, text);
}

void AudienceOutput::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    if (video_widget_ != nullptr) {
        video_widget_->setGeometry(rect());
    }
}

}  // namespace shekinah::media
