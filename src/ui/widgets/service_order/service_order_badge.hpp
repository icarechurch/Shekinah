#pragma once

#include "shekinah/domain/model.hpp"

#include <QColor>

class QPainter;
class QRectF;

namespace shekinah::ui::service_order_internal {

void draw_badge_glyph(QPainter& painter,
                      const QRectF& rect,
                      domain::ContentKind kind,
                      const QColor& color);

}  // namespace shekinah::ui::service_order_internal
