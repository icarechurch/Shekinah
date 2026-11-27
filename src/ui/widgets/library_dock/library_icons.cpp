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

}  // namespace library_internal
}  // namespace shekinah::ui
