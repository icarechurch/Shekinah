#include "shekinah/media/slide_renderer.hpp"

#include "slide_renderer_geometry.hpp"

#include <QPainter>

#include <algorithm>
#include <utility>

namespace shekinah::media {

using slide_renderer_internal::kCanonicalHeight;
using slide_renderer_internal::kCanonicalPadX;
using slide_renderer_internal::kCanonicalPadY;
using slide_renderer_internal::kCanonicalShadowOffset;
using slide_renderer_internal::kCanonicalWidth;

void SlideRenderer::set_content(SlideRenderData content) {
    content_ = std::move(content);
    background_image_ = {};
    background_image_size_ = {};
    text_layout_.reset();
    text_draw_origin_ = {};
    text_fits_height_ = true;
    text_layout_ready_ = false;
}

void SlideRenderer::paint(QPainter& painter, const QRect& viewport) {
    if (viewport.isEmpty()) {
        return;
    }

    painter.save();
    painter.setClipRect(viewport);
    ensure_background_image(viewport);
    if (!background_image_.isNull()) {
        if (content_.background_fit == domain::FitMode::stretch) {
            painter.drawImage(viewport, background_image_);
        } else {
            QSize draw_size = background_image_.size();
            draw_size.scale(
                viewport.size(),
                content_.background_fit == domain::FitMode::contain
                    ? Qt::KeepAspectRatio
                    : Qt::KeepAspectRatioByExpanding);
            const QRect draw_rect{
                viewport.center() - QPoint{draw_size.width() / 2, draw_size.height() / 2},
                draw_size};
            painter.drawImage(draw_rect, background_image_);
        }
    } else if (content_.checkerboard_fallback) {
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

    ensure_text_layout();
    if (text_layout_) {
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setRenderHint(QPainter::TextAntialiasing, true);

        const qreal scale = std::min(
            static_cast<qreal>(viewport.width()) / static_cast<qreal>(kCanonicalWidth),
            static_cast<qreal>(viewport.height()) / static_cast<qreal>(kCanonicalHeight));
        const qreal stage_w = static_cast<qreal>(kCanonicalWidth) * scale;
        const qreal stage_h = static_cast<qreal>(kCanonicalHeight) * scale;
        const qreal origin_x = static_cast<qreal>(viewport.x()) +
                               (static_cast<qreal>(viewport.width()) - stage_w) * 0.5;
        const qreal origin_y = static_cast<qreal>(viewport.y()) +
                               (static_cast<qreal>(viewport.height()) - stage_h) * 0.5;

        painter.translate(origin_x, origin_y);
        painter.scale(scale, scale);

        const QRectF clip_rect = text_fits_height_
            ? QRectF{static_cast<qreal>(kCanonicalPadX),
                     static_cast<qreal>(kCanonicalPadY),
                     static_cast<qreal>(kCanonicalWidth - 2 * kCanonicalPadX) + kCanonicalShadowOffset.x(),
                     static_cast<qreal>(kCanonicalHeight - 2 * kCanonicalPadY) + kCanonicalShadowOffset.y()}
            : QRectF{static_cast<qreal>(kCanonicalPadX),
                     static_cast<qreal>(kCanonicalPadY),
                     static_cast<qreal>(kCanonicalWidth - 2 * kCanonicalPadX),
                     static_cast<qreal>(kCanonicalHeight - 2 * kCanonicalPadY)};
        painter.setClipRect(clip_rect);

        painter.setPen(QColor{QStringLiteral("#080808")});
        text_layout_->draw(&painter, text_draw_origin_ + kCanonicalShadowOffset);
        painter.setPen(Qt::white);
        text_layout_->draw(&painter, text_draw_origin_);
    }
    painter.restore();
}

}  // namespace shekinah::media
