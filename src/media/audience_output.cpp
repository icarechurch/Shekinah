#include "shekinah/media/audience_output.hpp"

#include <QGuiApplication>
#include <QPainter>
#include <QScreen>

namespace shekinah::media {

AudienceOutput::AudienceOutput(QWidget* parent) : QWidget(parent) {
    setWindowTitle(QStringLiteral("Shekinah Audience Output"));
    setAttribute(Qt::WA_OpaquePaintEvent);
    resize(1280, 720);
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
    update();
}

void AudienceOutput::set_video_paused(bool paused) noexcept {
    video_paused_ = paused;
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

}  // namespace shekinah::media
