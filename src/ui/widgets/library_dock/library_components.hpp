#pragma once

#include "shekinah/ui/library_dock_widget.hpp"
#include "shekinah/ui/support/asset_drag.hpp"
#include "shekinah/ui/support/content_drag.hpp"
#include "shekinah/ui/support/elided_label.hpp"
#include "shekinah/ui/support/flow_layout.hpp"
#include "shekinah/ui/support/theme.hpp"

#include <QAbstractItemView>
#include <QAction>
#include <QButtonGroup>
#include <QFrame>
#include <QHBoxLayout>
#include <QIcon>
#include <QImageReader>
#include <QKeyEvent>
#include <QKeySequence>
#include <QLabel>
#include <QLineEdit>
#include <QListView>
#include <QListWidget>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QPushButton>
#include <QResizeEvent>
#include <QShortcut>
#include <QStackedWidget>
#include <QStyle>
#include <QStyledItemDelegate>
#include <QVBoxLayout>

#include <algorithm>
#include <utility>

#include <cstdint>
#include <functional>

namespace shekinah::ui {
namespace library_internal {


constexpr int kContentIdRole = Qt::UserRole + 1;
constexpr int kAssetKindRole = Qt::UserRole + 2;
constexpr int kAssetSizeRole = Qt::UserRole + 3;
constexpr int kContentKindRole = Qt::UserRole + 4;
constexpr int kSlideCountRole = Qt::UserRole + 5;
constexpr int kHasBgRole = Qt::UserRole + 6;
constexpr int kRelativePathRole = Qt::UserRole + 7;

constexpr int kThumbnailWidth = 168;
constexpr int kThumbnailHeight = 94;
constexpr int kAssetGridWidth = 192;
constexpr int kAssetGridHeight = 132;
constexpr int kContentGridWidth = 274;
constexpr int kContentGridHeight = 56;
constexpr int kThumbnailCacheCapKb = 4096;  // ~64 bounded 168x94 thumbnails max

enum class LibraryGlyph {
    search,
    assets_tab,
    songs_tab,
    sermons_tab,
    plus,
    import_doc,
    edit,
    float_dock,
    video_media,
    image_media,
};

inline QIcon make_library_icon(LibraryGlyph glyph, const QColor& color, int size = 14) {
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

inline void draw_content_badge_glyph(QPainter& p,
                                     const QRectF& rect,
                                     domain::ContentKind kind,
                                     const QColor& color) {
    p.save();
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setPen(QPen{color, 1.4, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin});
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

inline QString format_compact_bytes(std::uint64_t bytes) {
    if (bytes < 1024ULL) {
        return QStringLiteral("%1 B").arg(static_cast<qulonglong>(bytes));
    }
    if (bytes < 1024ULL * 1024ULL) {
        const double kb = static_cast<double>(bytes) / 1024.0;
        return QStringLiteral("%1 KB").arg(kb, 0, 'f', kb < 10.0 ? 1 : 0);
    }
    const double mb = static_cast<double>(bytes) / (1024.0 * 1024.0);
    return QStringLiteral("%1 MB").arg(mb, 0, 'f', 1);
}

inline QRect quick_add_rect_for_card(const QRect& card_rect) {
    return QRect{card_rect.right() - 48, card_rect.top() + (card_rect.height() - 20) / 2, 40, 20};
}

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

        // Top-left media kind badge
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

        // Top-right size pill
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

        // Thin thumbnail border
        painter->setPen(QPen{p.thumb_well_border, 1.0});
        painter->setBrush(Qt::NoBrush);
        painter->drawRoundedRect(thumb_well, 4.0, 4.0);

        // Filename footer
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
        subtitle += QStringLiteral(" Ã‚Â· %1 slide%2")
                        .arg(slide_count)
                        .arg(slide_count == 1 ? QString() : QStringLiteral("s"));
        if (is_song && has_bg) {
            subtitle += QStringLiteral(" Ã‚Â· Bg");
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

inline void refresh_widget_polish(QWidget* widget) {
    if (widget != nullptr && widget->style() != nullptr) {
        widget->style()->unpolish(widget);
        widget->style()->polish(widget);
    }
}

}  // namespace library_internal
}  // namespace shekinah::ui

