#include "service_order_delegate.hpp"

#include "service_order_badge.hpp"
#include "service_order_roles.hpp"
#include "service_order_row_surface.hpp"

#include "shekinah/ui/support/theme.hpp"

#include <QFont>
#include <QFontMetrics>
#include <QPainter>
#include <QStyle>
#include <QStyledItemDelegate>

#include <algorithm>

namespace shekinah::ui {
using namespace service_order_internal;
namespace {

class ServiceOrderItemDelegate final : public QStyledItemDelegate {
public:
    using QStyledItemDelegate::QStyledItemDelegate;

    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex&) const override {
        return {option.rect.width(), kRowHeight};
    }

    void paint(QPainter* painter,
               const QStyleOptionViewItem& option,
               const QModelIndex& index) const override {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing, true);

        const QRect bounds = option.rect;
        const bool selected = option.state.testFlag(QStyle::State_Selected);
        const bool hovered = option.state.testFlag(QStyle::State_MouseOver);
        const bool is_live = index.data(kIsLiveRole).toBool();
        const bool drop_target = index.data(kDropTargetRole).toBool();
        const auto kind = static_cast<domain::ContentKind>(index.data(kKindRole).toInt());
        const int slide_count = index.data(kSlideCountRole).toInt();
        const bool has_bg = index.data(kHasBgRole).toBool();
        const int ordinal = index.data(kOrdinalRole).toInt();
        const QString title = index.data(Qt::DisplayRole).toString();

        const auto& p = ThemeManager::instance().palette();

        const int right_occupied = paint_service_order_row_surface(
            *painter, option, bounds, p, selected, hovered, is_live, drop_target, ordinal);

        const QRectF badge_rect{
            static_cast<qreal>(bounds.left() + 10),
            static_cast<qreal>(bounds.top() + (bounds.height() - 26) / 2),
            26.0,
            26.0,
        };
        const bool is_song = (kind == domain::ContentKind::song);
        const QColor badge_fill = is_song ? p.song_badge_bg : p.sermon_badge_bg;
        const QColor badge_fg = is_song ? p.song_badge_fg : p.sermon_badge_fg;
        painter->setPen(QPen{is_song ? p.song_badge_border : p.sermon_badge_border, 1.0});
        painter->setBrush(badge_fill);
        painter->drawRoundedRect(badge_rect, 5.0, 5.0);
        draw_badge_glyph(*painter, badge_rect, kind, badge_fg);

        const int text_left = static_cast<int>(badge_rect.right()) + 9;
        const int text_width = std::max(0, bounds.right() - right_occupied - text_left);

        QFont title_font = option.font;
        title_font.setPixelSize(12);
        title_font.setWeight(selected || is_live ? QFont::DemiBold : QFont::Medium);
        painter->setFont(title_font);
        painter->setPen(selected || is_live ? p.text_selected : p.text_primary);
        const QFontMetrics title_fm{title_font};
        const QString elided_title = title_fm.elidedText(title, Qt::ElideRight, text_width);
        const QRect title_rect{text_left, bounds.top() + 7, text_width, 16};
        painter->drawText(title_rect, Qt::AlignVCenter | Qt::AlignLeft, elided_title);

        QString subtitle = is_song ? QStringLiteral("Song") : QStringLiteral("Sermon");
        if (slide_count > 0) {
            subtitle += QStringLiteral(" Ãƒâ€šÃ‚Â· %1 slide%2")
                            .arg(slide_count)
                            .arg(slide_count == 1 ? QString() : QStringLiteral("s"));
        }
        if (is_song && has_bg) {
            subtitle += QStringLiteral(" Ãƒâ€šÃ‚Â· Bg");
        }
        if (drop_target) {
            subtitle = QStringLiteral("Drop to apply background");
        }

        QFont sub_font = option.font;
        sub_font.setPixelSize(10);
        painter->setFont(sub_font);
        painter->setPen(drop_target ? p.drop_highlight
                                    : selected ? p.text_selected_sub : p.text_muted);
        const QFontMetrics sub_fm{sub_font};
        const QString elided_sub = sub_fm.elidedText(subtitle, Qt::ElideRight, text_width);
        const QRect sub_rect{text_left, bounds.top() + 24, text_width, 15};
        painter->drawText(sub_rect, Qt::AlignVCenter | Qt::AlignLeft, elided_sub);

        painter->restore();
    }
};

}  // namespace

namespace service_order_internal {

QAbstractItemDelegate* make_service_order_item_delegate(QObject* parent) {
    return new ServiceOrderItemDelegate{parent};
}

}  // namespace service_order_internal
}  // namespace shekinah::ui
