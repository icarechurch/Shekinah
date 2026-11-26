#include "shekinah/media/audience_output.hpp"

#include <QPainter>
#include <QResizeEvent>
#include <QVideoWidget>

namespace shekinah::media {

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
