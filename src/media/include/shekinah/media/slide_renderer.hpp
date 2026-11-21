#pragma once

#include "shekinah/domain/model.hpp"

#include <QImage>
#include <QRect>
#include <QString>

class QPainter;

namespace shekinah::media {

enum class SlideTextMode {
    preview,
    audience,
};

struct SlideRenderData {
    QString text;
    QString background_path;
    domain::FitMode background_fit{domain::FitMode::cover};
    bool show_text{true};
    bool checkerboard_fallback{true};
    SlideTextMode text_mode{SlideTextMode::preview};
};

class SlideRenderer final {
public:
    void set_content(SlideRenderData content);
    void paint(QPainter& painter, const QRect& viewport);

private:
    void ensure_background_image(const QRect& viewport);

    SlideRenderData content_;
    QImage background_image_;
    QSize background_image_size_;
};

}  // namespace shekinah::media
