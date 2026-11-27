#include "slide_renderer_background.hpp"

#include <QImageReader>
#include <QPainter>

#include <utility>

namespace shekinah::media {

void SlideBackgroundRenderer::set_content(
    QString path, domain::FitMode fit, bool checkerboard_fallback) {
    path_ = std::move(path);
    fit_ = fit;
    checkerboard_fallback_ = checkerboard_fallback;
    image_ = {};
    image_viewport_ = {};
}

void SlideBackgroundRenderer::paint(QPainter& painter, const QRect& viewport) {
    if (viewport.isEmpty()) {
        return;
    }

    ensure_image(viewport);
    if (!image_.isNull()) {
        if (fit_ == domain::FitMode::stretch) {
            painter.drawImage(viewport, image_);
        } else {
            QSize draw_size = image_.size();
            draw_size.scale(
                viewport.size(),
                fit_ == domain::FitMode::contain
                    ? Qt::KeepAspectRatio
                    : Qt::KeepAspectRatioByExpanding);
            const QRect draw_rect{
                viewport.center() - QPoint{draw_size.width() / 2, draw_size.height() / 2},
                draw_size};
            painter.drawImage(draw_rect, image_);
        }
    } else if (checkerboard_fallback_) {
        constexpr int tile_size = 16;
        for (int y = viewport.top(); y <= viewport.bottom(); y += tile_size) {
            for (int x = viewport.left(); x <= viewport.right(); x += tile_size) {
                const auto tile = ((x - viewport.left()) / tile_size +
                                   (y - viewport.top()) / tile_size) % 2;
                painter.fillRect(
                    QRect{x, y, tile_size, tile_size},
                    tile == 0 ? QColor{QStringLiteral("#111111")}
                              : QColor{QStringLiteral("#282828")});
            }
        }
    } else {
        painter.fillRect(viewport, QColor{QStringLiteral("#182337")});
    }
}

void SlideBackgroundRenderer::ensure_image(const QRect& viewport) {
    if (path_.isEmpty() || image_viewport_ == viewport.size()) {
        return;
    }
    image_viewport_ = viewport.size();
    image_ = {};

    QImageReader reader{path_};
    reader.setAutoTransform(true);
    QSize decoded_size = viewport.size();
    const auto source_size = reader.size();
    if (source_size.isValid() && fit_ != domain::FitMode::stretch) {
        decoded_size = source_size;
        decoded_size.scale(
            viewport.size(),
            fit_ == domain::FitMode::contain
                ? Qt::KeepAspectRatio
                : Qt::KeepAspectRatioByExpanding);
    }
    reader.setScaledSize(decoded_size);
    image_ = reader.read();
}

}  // namespace shekinah::media
