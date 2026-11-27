#pragma once

#include "shekinah/ui/support/theme_types.hpp"

class QPainter;
class QRect;
class QStyleOptionViewItem;

namespace shekinah::ui::service_order_internal {

int paint_service_order_row_surface(QPainter& painter,
                                    const QStyleOptionViewItem& option,
                                    const QRect& bounds,
                                    const ThemePalette& palette,
                                    bool selected,
                                    bool hovered,
                                    bool is_live,
                                    bool drop_target,
                                    int ordinal);

}  // namespace shekinah::ui::service_order_internal
