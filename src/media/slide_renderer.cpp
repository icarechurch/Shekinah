#include "shekinah/media/slide_renderer.hpp"

#include <QFontMetricsF>
#include <QGuiApplication>
#include <QImageReader>
#include <QPainter>
#include <QTextOption>

#include <algorithm>
#include <utility>

namespace shekinah::media {
namespace {

constexpr int kCanonicalWidth = 1920;
constexpr int kCanonicalHeight = 1080;
constexpr int kCanonicalPadX = kCanonicalWidth / 24;
constexpr int kCanonicalPadY = kCanonicalHeight / 20;
constexpr int kCanonicalMinFontPx = 24;
constexpr int kCanonicalMaxFontPx = kCanonicalHeight / 8;
constexpr QPointF kCanonicalShadowOffset{5.0, 5.0};

struct BuiltTextLayout {
    std::shared_ptr<QTextLayout> layout;
    qreal total_height{0.0};
    qreal max_line_width{0.0};
};

BuiltTextLayout build_text_layout(const QFont& base_font,
                                  const QString& text,
                                  qreal avail_width,
                                  int pixel_size,
                                  QTextOption::WrapMode wrap_mode) {
    QFont font = base_font;
    font.setPixelSize(pixel_size);

    auto layout = std::make_shared<QTextLayout>(text, font);
    layout->setCacheEnabled(true);
    QTextOption option;
    option.setAlignment(Qt::AlignHCenter);
    option.setWrapMode(wrap_mode);
    layout->setTextOption(option);

    const QFontMetricsF metrics{font};
    const qreal leading = std::max<qreal>(0.0, metrics.leading());
    qreal y = 0.0;
    qreal max_line_width = 0.0;

    layout->beginLayout();
    while (true) {
        QTextLine line = layout->createLine();
        if (!line.isValid()) {
            break;
        }
        line.setLineWidth(avail_width);
        if (y > 0.0) {
            y += leading;
        }
        line.setPosition(QPointF{0.0, y});
        y += line.height();
        max_line_width = std::max(max_line_width, line.naturalTextWidth());
    }
    layout->endLayout();

    return {std::move(layout), y, max_line_width};
}

}  // namespace

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

void SlideRenderer::ensure_text_layout() {
    if (text_layout_ready_) {
        return;
    }
    text_layout_ready_ = true;

    const QString trimmed = content_.text.trimmed();
    if (!content_.show_text || trimmed.isEmpty()) {
        return;
    }

    QString normalized = trimmed.toUpper();
    normalized.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
    normalized.replace(QLatin1Char('\r'), QLatin1Char('\n'));
    normalized.replace(QLatin1Char('\n'), QChar::LineSeparator);

    QFont base_font = QGuiApplication::instance() != nullptr ? QGuiApplication::font() : QFont{};
    base_font.setBold(true);
    base_font.setHintingPreference(QFont::PreferNoHinting);

    const qreal avail_w = static_cast<qreal>(kCanonicalWidth - 2 * kCanonicalPadX);
    const qreal avail_h = static_cast<qreal>(kCanonicalHeight - 2 * kCanonicalPadY);

    int low = kCanonicalMinFontPx;
    int high = kCanonicalMaxFontPx;
    BuiltTextLayout best;
    bool any_fit = false;

    while (low <= high) {
        const int mid = low + (high - low) / 2;
        auto candidate = build_text_layout(
            base_font, normalized, avail_w, mid, QTextOption::WordWrap);
        if (candidate.max_line_width <= avail_w && candidate.total_height <= avail_h) {
            best = std::move(candidate);
            any_fit = true;
            low = mid + 1;
        } else {
            high = mid - 1;
        }
    }

    if (!any_fit) {
        best = build_text_layout(
            base_font, normalized, avail_w, kCanonicalMinFontPx, QTextOption::WordWrap);
        if (best.max_line_width > avail_w) {
            best = build_text_layout(
                base_font,
                normalized,
                avail_w,
                kCanonicalMinFontPx,
                QTextOption::WrapAtWordBoundaryOrAnywhere);
        }
    }

    text_fits_height_ = best.total_height <= avail_h;
    const qreal y_offset = text_fits_height_ ? (avail_h - best.total_height) * 0.5 : 0.0;
    text_draw_origin_ = QPointF{
        static_cast<qreal>(kCanonicalPadX),
        static_cast<qreal>(kCanonicalPadY) + y_offset,
    };
    text_layout_ = std::move(best.layout);
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
