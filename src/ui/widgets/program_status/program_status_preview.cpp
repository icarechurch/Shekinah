#include "program_status_preview.hpp"

#include "program_status_preview_paint.hpp"

#include <QPainter>
#include <QPaintEvent>

#include <algorithm>
#include <cmath>
#include <utility>

namespace shekinah::ui {

ProgramSlidePreview::ProgramSlidePreview(QWidget* parent) : QFrame(parent) {
    setAttribute(Qt::WA_OpaquePaintEvent);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    setMinimumHeight(96);
}

bool ProgramSlidePreview::hasHeightForWidth() const { return true; }

int ProgramSlidePreview::heightForWidth(int width) const {
    return std::clamp(static_cast<int>(std::lround(static_cast<double>(width) * 9.0 / 16.0)),
                      96,
                      190);
}

void ProgramSlidePreview::set_mode(domain::ProgramMode mode) {
    mode_ = mode;
    update();
}

void ProgramSlidePreview::set_content(QString text,
                                      QString background_path,
                                      domain::FitMode background_fit) {
    text_ = std::move(text);
    background_path_ = std::move(background_path);
    background_fit_ = background_fit;
    renderer_.set_content(media::SlideRenderData{
        text_,
        background_path_,
        background_fit_,
        true,
        false,
    });
    setProperty("slide-text", text_);
    setProperty("slide-background-path", background_path_);
    update();
}

void ProgramSlidePreview::paintEvent(QPaintEvent*) {
    QPainter painter{this};
    painter.setRenderHint(QPainter::Antialiasing, true);
    paint_program_slide_preview(painter,
                                rect(),
                                mode_,
                                text_,
                                background_path_,
                                renderer_);
}

QString mode_text(domain::ProgramMode mode) {
    switch (mode) {
    case domain::ProgramMode::cue:
        return QStringLiteral("LIVE");
    case domain::ProgramMode::clear:
        return QStringLiteral("CLEAR");
    case domain::ProgramMode::black:
        return QStringLiteral("BLACK");
    }
    return QStringLiteral("UNKNOWN");
}

QString fit_mode_text(domain::FitMode fit) {
    switch (fit) {
    case domain::FitMode::contain:
        return QStringLiteral("Contain");
    case domain::FitMode::cover:
        return QStringLiteral("Cover");
    case domain::FitMode::stretch:
        return QStringLiteral("Stretch");
    }
    return QStringLiteral("Cover");
}

}  // namespace shekinah::ui
