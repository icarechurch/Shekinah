#include "library_components.hpp"

#include <QFontMetrics>
#include <QStyledItemDelegate>

namespace shekinah::ui {
namespace library_internal {

class AssetCardDelegate final : public QStyledItemDelegate {
public:
    using QStyledItemDelegate::QStyledItemDelegate;

    QSize sizeHint(const QStyleOptionViewItem&, const QModelIndex&) const override {
        return {kAssetGridWidth, kAssetGridHeight};
    }

    void paint(QPainter* painter,
               const QStyleOptionViewItem& option,
               const QModelIndex& index) const override {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing, true);

        const QRect card = option.rect.adjusted(4, 3, -4, -3);
        const bool selected = option.state.testFlag(QStyle::State_Selected);
        const bool hovered = option.state.testFlag(QStyle::State_MouseOver);
        const auto kind = static_cast<domain::AssetKind>(index.data(kAssetKindRole).toInt());
        const auto bytes = index.data(kAssetSizeRole).toULongLong();
        const QString title = index.data(Qt::DisplayRole).toString();
        const QIcon icon = qvariant_cast<QIcon>(index.data(Qt::DecorationRole));

        const auto& p = ThemeManager::instance().palette();
        const QColor card_bg = selected ? p.card_selected_bg
                               : hovered ? p.card_hover_bg
                                         : p.card_bg;
        const QColor card_border = selected ? p.accent
                                   : hovered ? p.border_hover
                                             : p.border_card;
        painter->setBrush(card_bg);
        painter->setPen(QPen{card_border, selected ? 1.5 : 1.0});
        painter->drawRoundedRect(card, 6.0, 6.0);

        const int thumb_w = std::min(kThumbnailWidth, std::max(80, card.width() - 10));
        const int thumb_h = std::min(kThumbnailHeight, std::max(45, thumb_w * 9 / 16));
        const QRect thumb_well{
            card.left() + (card.width() - thumb_w) / 2,
            card.top() + 5,
            thumb_w,
            thumb_h,
        };
        painter->save();
        QPainterPath clip_path;
        clip_path.addRoundedRect(thumb_well, 4.0, 4.0);
        painter->setClipPath(clip_path);
        painter->fillRect(thumb_well, p.thumb_well_bg);

        if (!icon.isNull()) {
            const QPixmap pixmap = icon.pixmap(thumb_well.size());
            if (!pixmap.isNull()) {
                const QSize fitted = pixmap.size().scaled(thumb_well.size(), Qt::KeepAspectRatio);
                const QRect target{
                    thumb_well.left() + (thumb_well.width() - fitted.width()) / 2,
                    thumb_well.top() + (thumb_well.height() - fitted.height()) / 2,
                    fitted.width(),
                    fitted.height(),
                };
                painter->drawPixmap(target, pixmap);
            }
        } else {
            const QColor placeholder_fg = kind == domain::AssetKind::video
                                              ? QColor{QStringLiteral("#a78bfa")}
                                              : p.icon_empty;
            const QRect glyph_rect{
                thumb_well.center().x() - 14,
                thumb_well.center().y() - 14,
                28,
                28,
            };
            make_library_icon(kind == domain::AssetKind::video ? LibraryGlyph::video_media
                                                               : LibraryGlyph::image_media,
                              placeholder_fg,
                              24)
                .paint(painter, glyph_rect);
        }

        const bool is_video = (kind == domain::AssetKind::video);
        const QRect kind_pill{thumb_well.left() + 5, thumb_well.top() + 5, 28, 15};
        painter->setPen(QPen{is_video ? p.video_badge_border : p.image_badge_border, 1.0});
        painter->setBrush(is_video ? p.video_badge_bg : p.image_badge_bg);
        painter->drawRoundedRect(kind_pill, 3.0, 3.0);
        QFont pill_font = option.font;
        pill_font.setPixelSize(8);
        pill_font.setBold(true);
        painter->setFont(pill_font);
        painter->setPen(is_video ? p.video_badge_fg : p.image_badge_fg);
        painter->drawText(kind_pill, Qt::AlignCenter, is_video ? QStringLiteral("VID") : QStringLiteral("IMG"));

        if (bytes > 0) {
            const QString size_text = format_compact_bytes(bytes);
            const QFontMetrics pill_fm{pill_font};
            const int pill_width = std::clamp(pill_fm.horizontalAdvance(size_text) + 10, 34, 58);
            const QRect size_pill{
                thumb_well.right() - pill_width - 5, thumb_well.top() + 5, pill_width, 15};
            painter->setPen(QPen{p.size_pill_border, 1.0});
            painter->setBrush(p.size_pill_bg);
            painter->drawRoundedRect(size_pill, 3.0, 3.0);
            painter->setPen(p.size_pill_fg);
            painter->drawText(size_pill, Qt::AlignCenter, size_text);
        }
        painter->restore();

        painter->setPen(QPen{p.thumb_well_border, 1.0});
        painter->setBrush(Qt::NoBrush);
        painter->drawRoundedRect(thumb_well, 4.0, 4.0);

        QFont label_font = option.font;
        label_font.setPixelSize(11);
        label_font.setWeight(selected ? QFont::DemiBold : QFont::Medium);
        painter->setFont(label_font);
        painter->setPen(selected ? p.text_selected : p.text_primary);
        const QRect text_rect{card.left() + 6, thumb_well.bottom() + 4, card.width() - 12, 18};
        const QFontMetrics fm{label_font};
        painter->drawText(text_rect,
                          Qt::AlignVCenter | Qt::AlignHCenter,
                          fm.elidedText(title, Qt::ElideMiddle, text_rect.width()));

        painter->restore();
    }
};

class ContentCardDelegate final : public QStyledItemDelegate {
public:
    using QStyledItemDelegate::QStyledItemDelegate;

    QSize sizeHint(const QStyleOptionViewItem&, const QModelIndex&) const override {
        return {kContentGridWidth, kContentGridHeight};
    }

    void paint(QPainter* painter,
               const QStyleOptionViewItem& option,
               const QModelIndex& index) const override {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing, true);

        const QRect card = option.rect.adjusted(3, 3, -3, -3);
        const bool selected = option.state.testFlag(QStyle::State_Selected);
        const bool hovered = option.state.testFlag(QStyle::State_MouseOver);
        const auto kind = static_cast<domain::ContentKind>(index.data(kContentKindRole).toInt());
        const int slide_count = index.data(kSlideCountRole).toInt();
        const bool has_bg = index.data(kHasBgRole).toBool();
        const QString title = index.data(Qt::DisplayRole).toString();

        const auto& p = ThemeManager::instance().palette();
        const QColor card_bg = selected ? p.card_selected_bg
                               : hovered ? p.card_hover_bg
                                         : p.card_bg;
        const QColor card_border = selected ? p.accent
                                   : hovered ? p.border_hover
                                             : p.border_card;
        painter->setBrush(card_bg);
        painter->setPen(QPen{card_border, selected ? 1.5 : 1.0});
        painter->drawRoundedRect(card, 6.0, 6.0);

        const bool is_song = (kind == domain::ContentKind::song);
        const QRectF badge_rect{
            static_cast<qreal>(card.left() + 9),
            static_cast<qreal>(card.top() + (card.height() - 28) / 2),
            28.0,
            28.0,
        };
        const QColor badge_fill = is_song ? p.song_badge_bg : p.sermon_badge_bg;
        const QColor badge_stroke = is_song ? p.song_badge_border : p.sermon_badge_border;
        const QColor badge_fg = is_song ? p.song_badge_fg : p.sermon_badge_fg;
        painter->setPen(QPen{badge_stroke, 1.0});
        painter->setBrush(badge_fill);
        painter->drawRoundedRect(badge_rect, 5.0, 5.0);
        draw_content_badge_glyph(*painter, badge_rect, kind, badge_fg);

        int right_reserved = 10;
        if (hovered || selected) {
            const QRect add_pill = quick_add_rect_for_card(card);
            painter->setPen(QPen{selected ? p.add_pill_selected_border : p.add_pill_border, 1.0});
            painter->setBrush(selected ? p.add_pill_selected_bg : p.add_pill_bg);
            painter->drawRoundedRect(add_pill, 4.0, 4.0);
            QFont pill_font = option.font;
            pill_font.setPixelSize(9);
            pill_font.setBold(true);
            painter->setFont(pill_font);
            painter->setPen(selected ? p.add_pill_selected_fg : p.add_pill_fg);
            painter->drawText(add_pill, Qt::AlignCenter, QStringLiteral("+ Add"));
            right_reserved = 56;
        }

        const int text_left = static_cast<int>(badge_rect.right()) + 9;
        const int text_width = std::max(0, card.right() - right_reserved - text_left);
        QFont title_font = option.font;
        title_font.setPixelSize(12);
        title_font.setWeight(selected ? QFont::DemiBold : QFont::Medium);
        painter->setFont(title_font);
        painter->setPen(selected ? p.text_selected : p.text_primary);
        const QFontMetrics title_fm{title_font};
        const QRect title_rect{text_left, card.top() + 7, text_width, 16};
        painter->drawText(title_rect,
                          Qt::AlignVCenter | Qt::AlignLeft,
                          title_fm.elidedText(title, Qt::ElideRight, text_width));

        QString subtitle = is_song ? QStringLiteral("Song") : QStringLiteral("Sermon");
        subtitle += QStringLiteral(" Ãƒâ€šÃ‚Â· %1 slide%2")
                        .arg(slide_count)
                        .arg(slide_count == 1 ? QString() : QStringLiteral("s"));
        if (is_song && has_bg) {
            subtitle += QStringLiteral(" Ãƒâ€šÃ‚Â· Bg");
        }
        QFont sub_font = option.font;
        sub_font.setPixelSize(10);
        painter->setFont(sub_font);
        painter->setPen(selected ? p.text_selected_sub : p.text_muted);
        const QFontMetrics sub_fm{sub_font};
        const QRect sub_rect{text_left, card.top() + 24, text_width, 15};
        painter->drawText(sub_rect,
                          Qt::AlignVCenter | Qt::AlignLeft,
                          sub_fm.elidedText(subtitle, Qt::ElideRight, text_width));

        painter->restore();
    }
};

QAbstractItemDelegate* make_asset_card_delegate(QObject* parent) {
    return new AssetCardDelegate{parent};
}

QAbstractItemDelegate* make_content_card_delegate(QObject* parent) {
    return new ContentCardDelegate{parent};
}

}  // namespace library_internal
}  // namespace shekinah::ui
