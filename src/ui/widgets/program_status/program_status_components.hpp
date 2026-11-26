#pragma once

#include "shekinah/ui/program_status_widget.hpp"

#include "shekinah/ui/support/theme.hpp"

#include <QIcon>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>

namespace shekinah::ui {
namespace {

enum class IconGlyph {
    clear_all,
    slide,
    media_image,
    blackout,
    display,
    sliders,
    chevron_down,
    previous,
    next,
    clear_layer,
    layers,
    list,
    plus,
    arrow_right,
};

[[maybe_unused]] QIcon make_glyph_icon(IconGlyph glyph, const QColor& color, int size = 18) {
    QPixmap pixmap{size * 2, size * 2};
    pixmap.fill(Qt::transparent);
    pixmap.setDevicePixelRatio(2.0);
    QPainter p{&pixmap};
    p.setRenderHint(QPainter::Antialiasing, true);
    QPen pen{color, 1.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin};
    p.setPen(pen);
    const qreal s = static_cast<qreal>(size);

    switch (glyph) {
    case IconGlyph::clear_all:
        p.drawLine(QPointF{s * 0.30, s * 0.30}, QPointF{s * 0.70, s * 0.70});
        p.drawLine(QPointF{s * 0.70, s * 0.30}, QPointF{s * 0.30, s * 0.70});
        break;
    case IconGlyph::slide:
        p.drawRoundedRect(QRectF{s * 0.14, s * 0.22, s * 0.72, s * 0.50}, 2.0, 2.0);
        p.drawLine(QPointF{s * 0.34, s * 0.82}, QPointF{s * 0.66, s * 0.82});
        p.drawLine(QPointF{s * 0.50, s * 0.72}, QPointF{s * 0.50, s * 0.82});
        break;
    case IconGlyph::media_image:
        p.drawRoundedRect(QRectF{s * 0.14, s * 0.18, s * 0.72, s * 0.64}, 2.0, 2.0);
        p.drawEllipse(QPointF{s * 0.34, s * 0.38}, s * 0.07, s * 0.07);
        p.drawLine(QPointF{s * 0.22, s * 0.72}, QPointF{s * 0.44, s * 0.50});
        p.drawLine(QPointF{s * 0.44, s * 0.50}, QPointF{s * 0.58, s * 0.64});
        p.drawLine(QPointF{s * 0.58, s * 0.64}, QPointF{s * 0.68, s * 0.54});
        p.drawLine(QPointF{s * 0.68, s * 0.54}, QPointF{s * 0.78, s * 0.72});
        break;
    case IconGlyph::blackout:
        p.drawRoundedRect(QRectF{s * 0.16, s * 0.22, s * 0.68, s * 0.52}, 2.0, 2.0);
        p.drawLine(QPointF{s * 0.22, s * 0.78}, QPointF{s * 0.78, s * 0.18});
        break;
    case IconGlyph::display:
        p.drawRoundedRect(QRectF{s * 0.14, s * 0.18, s * 0.72, s * 0.50}, 2.0, 2.0);
        p.drawLine(QPointF{s * 0.30, s * 0.80}, QPointF{s * 0.70, s * 0.80});
        p.drawLine(QPointF{s * 0.50, s * 0.68}, QPointF{s * 0.50, s * 0.80});
        p.drawRect(QRectF{s * 0.26, s * 0.30, s * 0.48, s * 0.26});
        break;
    case IconGlyph::sliders:
        p.drawLine(QPointF{s * 0.18, s * 0.30}, QPointF{s * 0.82, s * 0.30});
        p.drawLine(QPointF{s * 0.18, s * 0.52}, QPointF{s * 0.82, s * 0.52});
        p.drawLine(QPointF{s * 0.18, s * 0.74}, QPointF{s * 0.82, s * 0.74});
        p.setBrush(color);
        p.drawEllipse(QPointF{s * 0.36, s * 0.30}, 1.8, 1.8);
        p.drawEllipse(QPointF{s * 0.64, s * 0.52}, 1.8, 1.8);
        p.drawEllipse(QPointF{s * 0.44, s * 0.74}, 1.8, 1.8);
        break;
    case IconGlyph::chevron_down:
        p.drawLine(QPointF{s * 0.26, s * 0.38}, QPointF{s * 0.50, s * 0.64});
        p.drawLine(QPointF{s * 0.50, s * 0.64}, QPointF{s * 0.74, s * 0.38});
        break;
    case IconGlyph::previous: {
        p.drawLine(QPointF{s * 0.28, s * 0.26}, QPointF{s * 0.28, s * 0.74});
        QPainterPath tri;
        tri.moveTo(s * 0.72, s * 0.26);
        tri.lineTo(s * 0.38, s * 0.50);
        tri.lineTo(s * 0.72, s * 0.74);
        tri.closeSubpath();
        p.setBrush(color);
        p.drawPath(tri);
        break;
    }
    case IconGlyph::next: {
        p.drawLine(QPointF{s * 0.72, s * 0.26}, QPointF{s * 0.72, s * 0.74});
        QPainterPath tri;
        tri.moveTo(s * 0.28, s * 0.26);
        tri.lineTo(s * 0.62, s * 0.50);
        tri.lineTo(s * 0.28, s * 0.74);
        tri.closeSubpath();
        p.setBrush(color);
        p.drawPath(tri);
        break;
    }
    case IconGlyph::clear_layer:
        p.drawEllipse(QPointF{s * 0.50, s * 0.50}, s * 0.32, s * 0.32);
        p.drawLine(QPointF{s * 0.37, s * 0.37}, QPointF{s * 0.63, s * 0.63});
        p.drawLine(QPointF{s * 0.63, s * 0.37}, QPointF{s * 0.37, s * 0.63});
        break;
    case IconGlyph::layers: {
        QPainterPath top;
        top.moveTo(s * 0.50, s * 0.18);
        top.lineTo(s * 0.82, s * 0.36);
        top.lineTo(s * 0.50, s * 0.54);
        top.lineTo(s * 0.18, s * 0.36);
        top.closeSubpath();
        p.drawPath(top);
        p.drawLine(QPointF{s * 0.18, s * 0.54}, QPointF{s * 0.50, s * 0.70});
        p.drawLine(QPointF{s * 0.50, s * 0.70}, QPointF{s * 0.82, s * 0.54});
        p.drawLine(QPointF{s * 0.18, s * 0.70}, QPointF{s * 0.50, s * 0.86});
        p.drawLine(QPointF{s * 0.50, s * 0.86}, QPointF{s * 0.82, s * 0.70});
        break;
    }
    case IconGlyph::list:
        p.drawLine(QPointF{s * 0.34, s * 0.30}, QPointF{s * 0.82, s * 0.30});
        p.drawLine(QPointF{s * 0.34, s * 0.50}, QPointF{s * 0.82, s * 0.50});
        p.drawLine(QPointF{s * 0.34, s * 0.70}, QPointF{s * 0.82, s * 0.70});
        p.setBrush(color);
        p.drawEllipse(QPointF{s * 0.20, s * 0.30}, 1.3, 1.3);
        p.drawEllipse(QPointF{s * 0.20, s * 0.50}, 1.3, 1.3);
        p.drawEllipse(QPointF{s * 0.20, s * 0.70}, 1.3, 1.3);
        break;
    case IconGlyph::plus:
        p.drawLine(QPointF{s * 0.50, s * 0.22}, QPointF{s * 0.50, s * 0.78});
        p.drawLine(QPointF{s * 0.22, s * 0.50}, QPointF{s * 0.78, s * 0.50});
        break;
    case IconGlyph::arrow_right:
        p.drawLine(QPointF{s * 0.22, s * 0.50}, QPointF{s * 0.76, s * 0.50});
        p.drawLine(QPointF{s * 0.54, s * 0.28}, QPointF{s * 0.76, s * 0.50});
        p.drawLine(QPointF{s * 0.54, s * 0.72}, QPointF{s * 0.76, s * 0.50});
        break;
    }
    return QIcon{pixmap};
}

}  // namespace
}  // namespace shekinah::ui
