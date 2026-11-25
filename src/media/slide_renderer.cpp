#include "shekinah/media/slide_renderer.hpp"

#include <QFontMetrics>
#include <QImageReader>
#include <QPainter>

#include <algorithm>
#include <utility>

namespace shekinah::media {
namespace {

struct TextFitResult {
    int pixel_size{12};
    bool fits_height{true};
    bool fits_width{true};
};

TextFitResult fit_text_to_rect(const QFont& base_font,
                               const QRect& text_rect,
                               const QString& text,
                               int min_pixel_size,
                               int max_pixel_size) {
    const int clamped_max = std::max(min_pixel_size, max_pixel_size);
    const int avail_w = std::max(1, text_rect.width() - 1);
    const int avail_h = std::max(1, text_rect.height() - 1);
    const QRect measure_bounds{0, 0, avail_w, avail_h};

    auto measure_at = [&](int px) {
        QFont probe = base_font;
        probe.setPixelSize(px);
        const QFontMetrics fm{probe};
        return fm.boundingRect(measure_bounds, Qt::AlignCenter | Qt::TextWordWrap, text);
    };

    int low = min_pixel_size;
    int high = clamped_max;
    int best = min_pixel_size;
    bool any_fit = false;

    while (low <= high) {
        const int mid = low + (high - low) / 2;
        const QRect measured = measure_at(mid);
        if (measured.width() <= avail_w && measured.height() <= avail_h) {
            best = mid;
            any_fit = true;
            low = mid + 1;
        } else {
            high = mid - 1;
        }
    }

    if (any_fit) {
        return {best, true, true};
    }

    const QRect min_measured = measure_at(min_pixel_size);
    return {
        min_pixel_size,
        min_measured.height() <= avail_h,
        min_measured.width() <= avail_w,
    };
}

}  // namespace

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

    if (content_.show_text && !content_.text.trimmed().isEmpty()) {
        const bool is_audience = content_.text_mode == SlideTextMode::audience;
        const int pad_x = is_audience
            ? std::clamp(viewport.width() / 24, 8, 64)
            : std::clamp(viewport.width() / 18, 4, 14);
        const int pad_y = is_audience
            ? std::clamp(viewport.height() / 20, 6, 54)
            : std::clamp(viewport.height() / 16, 4, 12);
        const QRect padded = viewport.adjusted(pad_x, pad_y, -pad_x, -pad_y);
        const QRect text_rect = (padded.width() >= 12 && padded.height() >= 12) ? padded : viewport;

        QFont lyric_font = painter.font();
        lyric_font.setBold(true);
        const int max_font_size = is_audience
            ? std::clamp(viewport.height() / 8, 14, 96)
            : std::clamp(viewport.height() / 8, 8, 18);
        const int min_font_size = is_audience ? 12 : 6;
        const QString text = content_.text.trimmed().toUpper();
        const auto fit = fit_text_to_rect(lyric_font, text_rect, text, min_font_size, max_font_size);

        lyric_font.setPixelSize(fit.pixel_size);
        painter.setFont(lyric_font);
        int flags = Qt::TextWordWrap;
        flags |= fit.fits_height ? Qt::AlignCenter : (Qt::AlignTop | Qt::AlignHCenter);
        if (!fit.fits_width) {
            flags |= Qt::TextWrapAnywhere;
        }

        painter.setClipRect(text_rect);
        painter.setPen(QColor{QStringLiteral("#080808")});
        painter.drawText(text_rect.translated(1, 1), flags, text);
        painter.setPen(Qt::white);
        painter.drawText(text_rect, flags, text);
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
