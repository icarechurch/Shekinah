#include "library_components.hpp"

#include <QPainter>
#include <QPainterPath>
#include <QPixmap>

namespace shekinah::ui {
namespace library_internal {

QIcon make_library_icon(LibraryGlyph glyph, const QColor& color, int size) {
    QPixmap pixmap{size * 2, size * 2};
    pixmap.fill(Qt::transparent);
    pixmap.setDevicePixelRatio(2.0);
    QPainter p{&pixmap};
    p.setRenderHint(QPainter::Antialiasing, true);
    QPen pen{color, 1.45, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin};
    p.setPen(pen);
    const qreal s = static_cast<qreal>(size);

    switch (glyph) {
    case LibraryGlyph::search:
        p.drawEllipse(QPointF{s * 0.44, s * 0.44}, s * 0.25, s * 0.25);
        p.drawLine(QPointF{s * 0.62, s * 0.62}, QPointF{s * 0.82, s * 0.82});
        break;
    case LibraryGlyph::assets_tab:
    case LibraryGlyph::image_media:
        p.drawRoundedRect(QRectF{s * 0.14, s * 0.18, s * 0.72, s * 0.64}, 2.0, 2.0);
        p.drawEllipse(QPointF{s * 0.34, s * 0.38}, s * 0.06, s * 0.06);
        p.drawLine(QPointF{s * 0.22, s * 0.72}, QPointF{s * 0.44, s * 0.50});
        p.drawLine(QPointF{s * 0.44, s * 0.50}, QPointF{s * 0.58, s * 0.64});
        p.drawLine(QPointF{s * 0.58, s * 0.64}, QPointF{s * 0.68, s * 0.54});
        p.drawLine(QPointF{s * 0.68, s * 0.54}, QPointF{s * 0.78, s * 0.72});
        break;
    case LibraryGlyph::songs_tab:
        p.setBrush(color);
        p.drawEllipse(QPointF{s * 0.32, s * 0.72}, s * 0.11, s * 0.08);
        p.drawEllipse(QPointF{s * 0.68, s * 0.64}, s * 0.11, s * 0.08);
        p.setBrush(Qt::NoBrush);
        p.drawLine(QPointF{s * 0.42, s * 0.70}, QPointF{s * 0.42, s * 0.24});
        p.drawLine(QPointF{s * 0.78, s * 0.62}, QPointF{s * 0.78, s * 0.18});
        p.drawLine(QPointF{s * 0.42, s * 0.24}, QPointF{s * 0.78, s * 0.18});
        break;
    case LibraryGlyph::sermons_tab:
        p.drawRoundedRect(QRectF{s * 0.16, s * 0.20, s * 0.68, s * 0.46}, 2.0, 2.0);
        p.drawLine(QPointF{s * 0.30, s * 0.36}, QPointF{s * 0.70, s * 0.36});
        p.drawLine(QPointF{s * 0.30, s * 0.50}, QPointF{s * 0.58, s * 0.50});
        p.drawLine(QPointF{s * 0.50, s * 0.66}, QPointF{s * 0.50, s * 0.82});
        p.drawLine(QPointF{s * 0.34, s * 0.82}, QPointF{s * 0.66, s * 0.82});
        break;
    case LibraryGlyph::plus:
        p.drawLine(QPointF{s * 0.50, s * 0.22}, QPointF{s * 0.50, s * 0.78});
        p.drawLine(QPointF{s * 0.22, s * 0.50}, QPointF{s * 0.78, s * 0.50});
        break;
    case LibraryGlyph::import_doc:
        p.drawLine(QPointF{s * 0.50, s * 0.18}, QPointF{s * 0.50, s * 0.64});
        p.drawLine(QPointF{s * 0.32, s * 0.46}, QPointF{s * 0.50, s * 0.64});
        p.drawLine(QPointF{s * 0.68, s * 0.46}, QPointF{s * 0.50, s * 0.64});
        p.drawLine(QPointF{s * 0.20, s * 0.66}, QPointF{s * 0.20, s * 0.80});
        p.drawLine(QPointF{s * 0.20, s * 0.80}, QPointF{s * 0.80, s * 0.80});
        p.drawLine(QPointF{s * 0.80, s * 0.80}, QPointF{s * 0.80, s * 0.66});
        break;
    case LibraryGlyph::edit: {
        QPainterPath path;
        path.moveTo(s * 0.22, s * 0.78);
        path.lineTo(s * 0.26, s * 0.60);
        path.lineTo(s * 0.68, s * 0.18);
        path.lineTo(s * 0.82, s * 0.32);
        path.lineTo(s * 0.40, s * 0.74);
        path.closeSubpath();
        p.drawPath(path);
        p.drawLine(QPointF{s * 0.58, s * 0.28}, QPointF{s * 0.72, s * 0.42});
        break;
    }
    case LibraryGlyph::float_dock:
        p.drawRoundedRect(QRectF{s * 0.28, s * 0.18, s * 0.54, s * 0.54}, 1.5, 1.5);
        p.drawLine(QPointF{s * 0.18, s * 0.38}, QPointF{s * 0.18, s * 0.82});
        p.drawLine(QPointF{s * 0.18, s * 0.82}, QPointF{s * 0.62, s * 0.82});
        break;
    case LibraryGlyph::video_media: {
        p.drawRoundedRect(QRectF{s * 0.14, s * 0.22, s * 0.72, s * 0.56}, 2.0, 2.0);
        QPainterPath play;
        play.moveTo(s * 0.42, s * 0.36);
        play.lineTo(s * 0.65, s * 0.50);
        play.lineTo(s * 0.42, s * 0.64);
        play.closeSubpath();
        p.setBrush(color);
        p.drawPath(play);
        break;
    }
    }
    return QIcon{pixmap};
}

void draw_content_badge_glyph(QPainter& p,
                              const QRectF& rect,
                              domain::ContentKind kind,
                              const QColor& color) {
    p.save();
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setPen(QPen{color, 1.4, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin});
    const qreal x = rect.left();
    const qreal y = rect.top();
    const qreal w = rect.width();
    const qreal h = rect.height();
    if (kind == domain::ContentKind::song) {
        p.setBrush(color);
        p.drawEllipse(QPointF{x + w * 0.32, y + h * 0.72}, w * 0.11, h * 0.08);
        p.drawEllipse(QPointF{x + w * 0.67, y + h * 0.64}, w * 0.11, h * 0.08);
        p.setBrush(Qt::NoBrush);
        p.drawLine(QPointF{x + w * 0.42, y + h * 0.70}, QPointF{x + w * 0.42, y + h * 0.26});
        p.drawLine(QPointF{x + w * 0.77, y + h * 0.62}, QPointF{x + w * 0.77, y + h * 0.20});
        p.drawLine(QPointF{x + w * 0.42, y + h * 0.26}, QPointF{x + w * 0.77, y + h * 0.20});
    } else {
        p.drawRoundedRect(QRectF{x + w * 0.20, y + h * 0.22, w * 0.60, h * 0.42}, 1.5, 1.5);
        p.drawLine(QPointF{x + w * 0.33, y + h * 0.37}, QPointF{x + w * 0.67, y + h * 0.37});
        p.drawLine(QPointF{x + w * 0.33, y + h * 0.50}, QPointF{x + w * 0.56, y + h * 0.50});
        p.drawLine(QPointF{x + w * 0.50, y + h * 0.64}, QPointF{x + w * 0.50, y + h * 0.78});
        p.drawLine(QPointF{x + w * 0.36, y + h * 0.78}, QPointF{x + w * 0.64, y + h * 0.78});
    }
    p.restore();
}

QString format_compact_bytes(std::uint64_t bytes) {
    if (bytes < 1024ULL) {
        return QStringLiteral("%1 B").arg(static_cast<qulonglong>(bytes));
    }
    if (bytes < 1024ULL * 1024ULL) {
        const double kb = static_cast<double>(bytes) / 1024.0;
        return QStringLiteral("%1 KB").arg(kb, 0, 'f', kb < 10.0 ? 1 : 0);
    }
    const double mb = static_cast<double>(bytes) / (1024.0 * 1024.0);
    return QStringLiteral("%1 MB").arg(mb, 0, 'f', 1);
}

QRect quick_add_rect_for_card(const QRect& card_rect) {
    return QRect{card_rect.right() - 48, card_rect.top() + (card_rect.height() - 20) / 2, 40, 20};
}

void refresh_widget_polish(QWidget* widget) {
    if (widget != nullptr && widget->style() != nullptr) {
        widget->style()->unpolish(widget);
        widget->style()->polish(widget);
    }
}

}  // namespace library_internal
}  // namespace shekinah::ui
