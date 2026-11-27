#pragma once

#include <QColor>
#include <QIcon>

namespace shekinah::ui::service_order_internal {

enum class OrderGlyph {
    plus,
    chevron_up,
    chevron_down,
    edit,
    trash,
    song,
    sermon,
    empty_stack,
};

QIcon make_order_icon(OrderGlyph glyph, const QColor& color, int size = 14);

}  // namespace shekinah::ui::service_order_internal
