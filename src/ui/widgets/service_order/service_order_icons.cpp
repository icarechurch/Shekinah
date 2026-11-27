#include "service_order_icons.hpp"

#include <QPainter>
#include <QPainterPath>
#include <QPixmap>

namespace shekinah::ui::service_order_internal {

QIcon make_order_icon(OrderGlyph glyph, const QColor& color, int size) {
    QPixmap pixmap{size * 2, size * 2};
    pixmap.fill(Qt::transparent);
    pixmap.setDevicePixelRatio(2.0);
    QPainter p{&pixmap};
    p.setRenderHint(QPainter::Antialiasing, true);
    QPen pen{color, 1.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin};
    p.setPen(pen);
    const qreal s = static_cast<qreal>(size);

    switch (glyph) {
    case OrderGlyph::plus:
        p.drawLine(QPointF{s * 0.50, s * 0.22}, QPointF{s * 0.50, s * 0.78});
        p.drawLine(QPointF{s * 0.22, s * 0.50}, QPointF{s * 0.78, s * 0.50});
        break;
    case OrderGlyph::chevron_up:
        p.drawLine(QPointF{s * 0.24, s * 0.62}, QPointF{s * 0.50, s * 0.34});
        p.drawLine(QPointF{s * 0.50, s * 0.34}, QPointF{s * 0.76, s * 0.62});
        break;
    case OrderGlyph::chevron_down:
        p.drawLine(QPointF{s * 0.24, s * 0.38}, QPointF{s * 0.50, s * 0.66});
        p.drawLine(QPointF{s * 0.50, s * 0.66}, QPointF{s * 0.76, s * 0.38});
        break;
    case OrderGlyph::edit: {
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
    case OrderGlyph::trash:
        p.drawLine(QPointF{s * 0.20, s * 0.28}, QPointF{s * 0.80, s * 0.28});
        p.drawLine(QPointF{s * 0.38, s * 0.18}, QPointF{s * 0.62, s * 0.18});
        p.drawRoundedRect(QRectF{s * 0.28, s * 0.28, s * 0.44, s * 0.54}, 1.5, 1.5);
        p.drawLine(QPointF{s * 0.42, s * 0.42}, QPointF{s * 0.42, s * 0.68});
        p.drawLine(QPointF{s * 0.58, s * 0.42}, QPointF{s * 0.58, s * 0.68});
        break;
    case OrderGlyph::song:
        p.setBrush(color);
        p.drawEllipse(QPointF{s * 0.32, s * 0.72}, s * 0.12, s * 0.09);
        p.drawEllipse(QPointF{s * 0.68, s * 0.64}, s * 0.12, s * 0.09);
        p.setBrush(Qt::NoBrush);
        p.drawLine(QPointF{s * 0.43, s * 0.70}, QPointF{s * 0.43, s * 0.24});
        p.drawLine(QPointF{s * 0.79, s * 0.62}, QPointF{s * 0.79, s * 0.18});
        p.drawLine(QPointF{s * 0.43, s * 0.24}, QPointF{s * 0.79, s * 0.18});
        break;
    case OrderGlyph::sermon:
        p.drawRoundedRect(QRectF{s * 0.18, s * 0.20, s * 0.64, s * 0.46}, 2.0, 2.0);
        p.drawLine(QPointF{s * 0.32, s * 0.36}, QPointF{s * 0.68, s * 0.36});
        p.drawLine(QPointF{s * 0.32, s * 0.50}, QPointF{s * 0.58, s * 0.50});
        p.drawLine(QPointF{s * 0.50, s * 0.66}, QPointF{s * 0.50, s * 0.82});
        p.drawLine(QPointF{s * 0.34, s * 0.82}, QPointF{s * 0.66, s * 0.82});
        break;
    case OrderGlyph::empty_stack:
        p.drawRoundedRect(QRectF{s * 0.18, s * 0.22, s * 0.64, s * 0.24}, 2.5, 2.5);
        p.drawRoundedRect(QRectF{s * 0.18, s * 0.54, s * 0.64, s * 0.24}, 2.5, 2.5);
        break;
    }
    return QIcon{pixmap};
}

}  // namespace shekinah::ui::service_order_internal
