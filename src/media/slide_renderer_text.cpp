#include "slide_renderer_text.hpp"

#include "slide_renderer_geometry.hpp"

#include <QFontMetricsF>
#include <QGuiApplication>
#include <QPainter>
#include <QTextLayout>
#include <QTextOption>

#include <algorithm>
#include <utility>

namespace shekinah::media {
namespace {

using slide_renderer_internal::kCanonicalHeight;
using slide_renderer_internal::kCanonicalMaxFontPx;
using slide_renderer_internal::kCanonicalMinFontPx;
using slide_renderer_internal::kCanonicalPadX;
using slide_renderer_internal::kCanonicalPadY;
using slide_renderer_internal::kCanonicalShadowOffset;
using slide_renderer_internal::kCanonicalWidth;

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

void SlideTextRenderer::set_content(QString text, bool show_text) {
    text_ = std::move(text);
    show_text_ = show_text;
    text_layout_.reset();
    text_draw_origin_ = {};
    text_fits_height_ = true;
    text_layout_ready_ = false;
}

void SlideTextRenderer::paint(QPainter& painter, const QRect& viewport) {
    ensure_text_layout();
    if (!text_layout_) {
        return;
    }

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

void SlideTextRenderer::ensure_text_layout() {
    if (text_layout_ready_) {
        return;
    }
    text_layout_ready_ = true;

    const QString trimmed = text_.trimmed();
    if (!show_text_ || trimmed.isEmpty()) {
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

}  // namespace shekinah::media
