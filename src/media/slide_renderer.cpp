#include "shekinah/media/slide_renderer.hpp"

#include <QImageReader>
#include <QPainter>

#include <algorithm>
#include <utility>

namespace shekinah::media {

void SlideRenderer::set_content(SlideRenderData content) {
    content_ = std::move(content);
    background_image_ = {};
    background_image_size_ = {};
}

void SlideRenderer::paint(QPainter& painter, const QRect& viewport) {
    if (viewport.isEmpty()) {
        return;
    }

    painter.save();
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
        for (int y = viewport.top(); y < viewport.bottom(); y += tile_size) {
            for (int x = viewport.left(); x < viewport.right(); x += tile_size) {
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

    if (content_.show_text && !content_.text.trimmed().isEmpty()) {
        QFont lyric_font = painter.font();
        lyric_font.setPixelSize(std::clamp(viewport.height() / 10, 11, 18));
        lyric_font.setBold(true);
        painter.setFont(lyric_font);
        const auto text = content_.text.toUpper();
        const auto text_rect = viewport.adjusted(20, 20, -20, -20);
        painter.setPen(QColor{QStringLiteral("#080808")});
        painter.drawText(text_rect.translated(1, 1), Qt::AlignCenter | Qt::TextWordWrap, text);
        painter.setPen(Qt::white);
        painter.drawText(text_rect, Qt::AlignCenter | Qt::TextWordWrap, text);
    }
    painter.restore();
}

void SlideRenderer::ensure_background_image(const QRect& viewport) {
    if (content_.background_path.isEmpty() || background_image_size_ == viewport.size()) {
        return;
    }
    background_image_size_ = viewport.size();
    background_image_ = {};

    QImageReader reader{content_.background_path};
    reader.setAutoTransform(true);
    QSize decoded_size = viewport.size();
    const auto source_size = reader.size();
    if (source_size.isValid() && content_.background_fit != domain::FitMode::stretch) {
        decoded_size = source_size;
        decoded_size.scale(
            viewport.size(),
            content_.background_fit == domain::FitMode::contain
                ? Qt::KeepAspectRatio
                : Qt::KeepAspectRatioByExpanding);
    }
    reader.setScaledSize(decoded_size);
    background_image_ = reader.read();
}

}  // namespace shekinah::media
