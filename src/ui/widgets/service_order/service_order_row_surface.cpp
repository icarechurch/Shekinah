#include "service_order_row_surface.hpp"

#include <QFont>
#include <QPainter>
#include <QStyle>
#include <QStyleOptionViewItem>

namespace shekinah::ui::service_order_internal {

int paint_service_order_row_surface(QPainter& painter,
                                    const QStyleOptionViewItem& option,
                                    const QRect& bounds,
                                    const ThemePalette& p,
                                    bool selected,
                                    bool hovered,
                                    bool is_live,
                                    bool drop_target,
                                    int ordinal) {
    QColor bg_color{p.panel_bg};
    if (drop_target) {
        bg_color = p.card_drop_bg;
    } else if (selected) {
        bg_color = p.order_selected_bg;
    } else if (hovered) {
        bg_color = p.list_hover_bg;
    }
    painter.fillRect(bounds, bg_color);

    painter.setPen(p.border_divider);
    painter.drawLine(bounds.bottomLeft(), bounds.bottomRight());

    if (is_live) {
        painter.fillRect(QRect{bounds.left(), bounds.top() + 3, 3, bounds.height() - 6},
                         p.live_fg);
    } else if (selected) {
        painter.fillRect(QRect{bounds.left(), bounds.top() + 3, 3, bounds.height() - 6},
                         p.accent);
    }

    const int right_padding = 8;
    int right_occupied = right_padding;
    if (is_live) {
        const QRect live_pill{bounds.right() - 40,
                              bounds.top() + (bounds.height() - 16) / 2,
                              32,
                              16};
        painter.setPen(QPen{p.live_pill_border, 1.0});
        painter.setBrush(p.live_pill_bg);
        painter.drawRoundedRect(live_pill, 3.0, 3.0);
        QFont pill_font = option.font;
        pill_font.setPixelSize(9);
        pill_font.setBold(true);
        painter.setFont(pill_font);
        painter.setPen(p.live_fg);
        painter.drawText(live_pill, Qt::AlignCenter, QStringLiteral("LIVE"));
        right_occupied = 46;
    } else if (ordinal > 0) {
        const QString ord_text = QStringLiteral("%1").arg(ordinal, 2, 10, QLatin1Char('0'));
        QFont ord_font = option.font;
        ord_font.setPixelSize(10);
        ord_font.setWeight(QFont::Medium);
        painter.setFont(ord_font);
        painter.setPen(selected ? p.text_selected_sub : p.text_subtle);
        const QRect ord_rect{bounds.right() - 28, bounds.top(), 20, bounds.height()};
        painter.drawText(ord_rect, Qt::AlignVCenter | Qt::AlignRight, ord_text);
        right_occupied = 32;
    }
    return right_occupied;
}

}  // namespace shekinah::ui::service_order_internal
