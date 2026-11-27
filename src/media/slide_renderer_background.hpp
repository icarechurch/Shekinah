#pragma once

#include "shekinah/domain/model.hpp"

#include <QImage>
#include <QRect>
#include <QString>

class QPainter;

namespace shekinah::media {

class SlideBackgroundRenderer final {
public:
    void set_content(QString path, domain::FitMode fit, bool checkerboard_fallback);
    void paint(QPainter& painter, const QRect& viewport);

private:
    void ensure_image(const QRect& viewport);

    QString path_;
    domain::FitMode fit_{domain::FitMode::cover};
    bool checkerboard_fallback_{true};
    QImage image_;
    QSize image_viewport_;
};

}  // namespace shekinah::media
