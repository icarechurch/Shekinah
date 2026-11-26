#pragma once

#include "shekinah/ui/service_order_widget.hpp"
#include "shekinah/ui/support/asset_drag.hpp"
#include "shekinah/ui/support/content_drag.hpp"
#include "shekinah/ui/support/elided_label.hpp"
#include "shekinah/ui/support/theme.hpp"

#include <QAbstractItemView>
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QFrame>
#include <QHBoxLayout>
#include <QIcon>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMenu>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QPushButton>
#include <QResizeEvent>
#include <QStyledItemDelegate>
#include <QVBoxLayout>

#include <algorithm>

namespace shekinah::ui {
namespace {

constexpr int kKindRole = Qt::UserRole + 1;
constexpr int kSlideCountRole = Qt::UserRole + 2;
constexpr int kHasBgRole = Qt::UserRole + 3;
constexpr int kIsLiveRole = Qt::UserRole + 4;
constexpr int kOrdinalRole = Qt::UserRole + 5;
constexpr int kDropTargetRole = Qt::UserRole + 6;
constexpr int kRowHeight = 46;

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

[[maybe_unused]] QIcon make_order_icon(OrderGlyph glyph, const QColor& color, int size = 14) {
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

void draw_badge_glyph(QPainter& p, const QRectF& rect, domain::ContentKind kind, const QColor& color) {
    p.save();
    p.setRenderHint(QPainter::Antialiasing, true);
    QPen pen{color, 1.4, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin};
    p.setPen(pen);
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

        // Row background
        QColor bg_color{p.panel_bg};
        if (drop_target) {
            bg_color = p.card_drop_bg;
        } else if (selected) {
            bg_color = p.order_selected_bg;
        } else if (hovered) {
            bg_color = p.list_hover_bg;
        }
        painter->fillRect(bounds, bg_color);

        // Bottom hairline divider
        painter->setPen(p.border_divider);
        painter->drawLine(bounds.bottomLeft(), bounds.bottomRight());

        // Left status / selection accent bar
        if (is_live) {
            painter->fillRect(QRect{bounds.left(), bounds.top() + 3, 3, bounds.height() - 6},
                              p.live_fg);
        } else if (selected) {
            painter->fillRect(QRect{bounds.left(), bounds.top() + 3, 3, bounds.height() - 6},
                              p.accent);
        }

        // Drop target highlight outline
        if (drop_target) {
            painter->setPen(QPen{p.drop_highlight, 1.5});
            painter->drawRoundedRect(bounds.adjusted(2, 2, -2, -2), 4.0, 4.0);
        }

        // Content type icon badge (left)
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

        // Right-side pill ("LIVE" or ordinal "#01")
        const int right_padding = 8;
        int right_occupied = right_padding;
        if (is_live) {
            const QRect live_pill{bounds.right() - 40, bounds.top() + (bounds.height() - 16) / 2, 32, 16};
            painter->setPen(QPen{p.live_pill_border, 1.0});
            painter->setBrush(p.live_pill_bg);
            painter->drawRoundedRect(live_pill, 3.0, 3.0);
            QFont pill_font = option.font;
            pill_font.setPixelSize(9);
            pill_font.setBold(true);
            painter->setFont(pill_font);
            painter->setPen(p.live_fg);
            painter->drawText(live_pill, Qt::AlignCenter, QStringLiteral("LIVE"));
            right_occupied = 46;
        } else if (ordinal > 0) {
            const QString ord_text = QStringLiteral("%1").arg(ordinal, 2, 10, QLatin1Char('0'));
            QFont ord_font = option.font;
            ord_font.setPixelSize(10);
            ord_font.setWeight(QFont::Medium);
            painter->setFont(ord_font);
            painter->setPen(selected ? p.text_selected_sub : p.text_subtle);
            const QRect ord_rect{bounds.right() - 28, bounds.top(), 20, bounds.height()};
            painter->drawText(ord_rect, Qt::AlignVCenter | Qt::AlignRight, ord_text);
            right_occupied = 32;
        }

        // Title + Metadata text region
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
            subtitle += QStringLiteral(" Â· %1 slide%2")
                            .arg(slide_count)
                            .arg(slide_count == 1 ? QString() : QStringLiteral("s"));
        }
        if (is_song && has_bg) {
            subtitle += QStringLiteral(" Â· Bg");
        }
        if (drop_target) {
            subtitle = QStringLiteral("Drop to apply background");
        }

        QFont sub_font = option.font;
        sub_font.setPixelSize(10);
        painter->setFont(sub_font);
        painter->setPen(drop_target
                            ? p.drop_highlight
                            : selected ? p.text_selected_sub
                                       : p.text_muted);
        const QFontMetrics sub_fm{sub_font};
        const QString elided_sub = sub_fm.elidedText(subtitle, Qt::ElideRight, text_width);
        const QRect sub_rect{text_left, bounds.top() + 24, text_width, 15};
        painter->drawText(sub_rect, Qt::AlignVCenter | Qt::AlignLeft, elided_sub);

        painter->restore();
    }
};

}  // namespace
}  // namespace shekinah::ui
