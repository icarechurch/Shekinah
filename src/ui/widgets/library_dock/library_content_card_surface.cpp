#include "library_components.hpp"

namespace shekinah::ui {
namespace library_internal {

int paint_content_card_surface(QPainter& painter,
                               const QStyleOptionViewItem& option,
                               const QRect& card,
                               bool selected,
                               bool hovered) {
    const auto& p = ThemeManager::instance().palette();
    const QColor card_bg = selected ? p.card_selected_bg
                           : hovered ? p.card_hover_bg
                                     : p.card_bg;
    const QColor card_border = selected ? p.accent
                               : hovered ? p.border_hover
                                         : p.border_card;
    painter.setBrush(card_bg);
    painter.setPen(QPen{card_border, selected ? 1.5 : 1.0});
    painter.drawRoundedRect(card, 6.0, 6.0);

    int right_reserved = 10;
    if (hovered || selected) {
        const QRect add_pill = quick_add_rect_for_card(card);
        painter.setPen(QPen{selected ? p.add_pill_selected_border : p.add_pill_border, 1.0});
        painter.setBrush(selected ? p.add_pill_selected_bg : p.add_pill_bg);
        painter.drawRoundedRect(add_pill, 4.0, 4.0);
        QFont pill_font = option.font;
        pill_font.setPixelSize(9);
        pill_font.setBold(true);
        painter.setFont(pill_font);
        painter.setPen(selected ? p.add_pill_selected_fg : p.add_pill_fg);
        painter.drawText(add_pill, Qt::AlignCenter, QStringLiteral("+ Add"));
        right_reserved = 56;
    }
    return right_reserved;
}

}  // namespace library_internal
}  // namespace shekinah::ui
