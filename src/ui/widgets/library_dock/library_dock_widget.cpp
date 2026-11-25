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

namespace shekinah::ui {
namespace {

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

QIcon make_library_icon(LibraryGlyph glyph, const QColor& color, int size = 14) {
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

void draw_content_badge_glyph(QPainter& p,
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

QString format_compact_bytes(std::uint64_t bytes) {
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

QRect quick_add_rect_for_card(const QRect& card_rect) {
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
        subtitle += QStringLiteral(" · %1 slide%2")
                        .arg(slide_count)
                        .arg(slide_count == 1 ? QString() : QStringLiteral("s"));
        if (is_song && has_bg) {
            subtitle += QStringLiteral(" · Bg");
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

class AssetListWidget final : public QListWidget {
public:
    using QListWidget::QListWidget;

protected:
    QMimeData* mimeData(const QList<QListWidgetItem*>& items) const override {
        if (items.size() != 1 || items.front() == nullptr) {
            return nullptr;
        }
        const auto id = items.front()->data(kContentIdRole).toString();
        return id.isEmpty() ? nullptr : make_asset_mime_data(domain::AssetId{id.toStdString()});
    }
};

class ContentListWidget final : public QListWidget {
public:
    ContentListWidget(domain::ContentKind kind, QWidget* parent) : QListWidget(parent), kind_(kind) {}

    void set_action_handlers(std::function<void(QListWidgetItem*)> add_handler,
                             std::function<void(QListWidgetItem*)> edit_handler) {
        add_handler_ = std::move(add_handler);
        edit_handler_ = std::move(edit_handler);
    }

protected:
    QMimeData* mimeData(const QList<QListWidgetItem*>& items) const override {
        if (items.size() != 1 || items.front() == nullptr) {
            return nullptr;
        }
        const auto id = items.front()->data(kContentIdRole).toString();
        if (id.isEmpty()) {
            return nullptr;
        }
        return make_content_mime_data(kind_ == domain::ContentKind::song
                                          ? domain::ContentRef::song(id.toStdString())
                                          : domain::ContentRef::sermon(id.toStdString()));
    }

    void mouseReleaseEvent(QMouseEvent* event) override {
        if (event->button() == Qt::LeftButton && add_handler_) {
            const QPoint pos = event->position().toPoint();
            if (auto* clicked = itemAt(pos); clicked != nullptr) {
                const QRect card = visualItemRect(clicked).adjusted(3, 3, -3, -3);
                if (quick_add_rect_for_card(card).contains(pos)) {
                    QListWidget::mouseReleaseEvent(event);
                    add_handler_(clicked);
                    return;
                }
            }
        }
        QListWidget::mouseReleaseEvent(event);
    }

    void keyPressEvent(QKeyEvent* event) override {
        if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
            if (auto* item = currentItem(); item != nullptr) {
                if (event->modifiers().testFlag(Qt::ControlModifier)) {
                    if (edit_handler_) {
                        edit_handler_(item);
                        event->accept();
                        return;
                    }
                } else if (add_handler_) {
                    add_handler_(item);
                    event->accept();
                    return;
                }
            }
        }
        QListWidget::keyPressEvent(event);
    }

private:
    domain::ContentKind kind_;
    std::function<void(QListWidgetItem*)> add_handler_;
    std::function<void(QListWidgetItem*)> edit_handler_;
};

void refresh_widget_polish(QWidget* widget) {
    if (widget != nullptr && widget->style() != nullptr) {
        widget->style()->unpolish(widget);
        widget->style()->polish(widget);
    }
}

}  // namespace

LibraryDockWidget::LibraryDockWidget(QWidget* parent)
    : QDockWidget(parent),
      shared_video_icon_(
          make_library_icon(LibraryGlyph::video_media, QColor{QStringLiteral("#a78bfa")}, 32)),
      thumbnail_cache_(kThumbnailCacheCapKb) {
    setObjectName(QStringLiteral("library-dock"));
    setWindowTitle(QStringLiteral("Library"));
    setAllowedAreas(Qt::BottomDockWidgetArea | Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable |
                QDockWidget::DockWidgetClosable);

    // 1. Sleek custom dock title bar (keeps dock draggable and floatable)
    auto* title_bar = new QWidget{this};
    title_bar->setObjectName(QStringLiteral("library-dock-header"));
    title_bar->setFixedHeight(26);
    auto* title_layout = new QHBoxLayout(title_bar);
    title_layout->setContentsMargins(10, 0, 6, 0);
    title_layout->setSpacing(6);

    auto* title_label = new QLabel{QStringLiteral("LIBRARY"), title_bar};
    title_label->setObjectName(QStringLiteral("library-dock-title"));
    title_layout->addWidget(title_label);

    header_count_badge_ = new QLabel{QStringLiteral("0 ITEMS"), title_bar};
    header_count_badge_->setObjectName(QStringLiteral("library-total-count"));
    header_count_badge_->setFixedHeight(18);
    header_count_badge_->setAlignment(Qt::AlignCenter);
    title_layout->addWidget(header_count_badge_, 0, Qt::AlignVCenter);

    auto* hint_label = new ElidedLabel{
        QStringLiteral("Drag Assets to Slides/Orders  ·  Double-click Song/Sermon to add"),
        title_bar};
    hint_label->setObjectName(QStringLiteral("library-dock-hint"));
    hint_label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    title_layout->addWidget(hint_label, 1);

    float_toggle_button_ = new QPushButton{title_bar};
    float_toggle_button_->setObjectName(QStringLiteral("library-float-toggle"));
    float_toggle_button_->setFixedSize(20, 20);
    float_toggle_button_->setCursor(Qt::PointingHandCursor);
    float_toggle_button_->setToolTip(QStringLiteral("Float or Dock Library"));
    float_toggle_button_->setIconSize(QSize{12, 12});
    QObject::connect(float_toggle_button_, &QPushButton::clicked, this, [this] {
        setFloating(!isFloating());
    });
    title_layout->addWidget(float_toggle_button_);
    setTitleBarWidget(title_bar);

    // 2. Main container
    auto* container = new QWidget{this};
    auto* container_layout = new QVBoxLayout(container);
    container_layout->setContentsMargins(0, 0, 0, 0);
    container_layout->setSpacing(0);

    // 3. Responsive ProPresenter toolbar (Tabs + Search + Media Filters + Grouped Actions)
    auto* toolbar = new QWidget{container};
    toolbar->setObjectName(QStringLiteral("library-toolbar"));
    QSizePolicy toolbar_policy{QSizePolicy::Preferred, QSizePolicy::Preferred};
    toolbar_policy.setHeightForWidth(true);
    toolbar->setSizePolicy(toolbar_policy);
    auto* toolbar_layout = new FlowLayout{toolbar, 0, 6, 4};
    toolbar_layout->setContentsMargins(8, 4, 8, 4);

    auto* tab_bar = new QWidget{toolbar};
    tab_bar->setObjectName(QStringLiteral("library-tabs"));
    tab_bar->setFixedHeight(26);
    auto* tab_layout = new QHBoxLayout(tab_bar);
    tab_layout->setContentsMargins(2, 2, 2, 2);
    tab_layout->setSpacing(2);

    tab_group_ = new QButtonGroup{this};
    tab_group_->setExclusive(true);

    const auto make_tab_btn = [&](const QString& label,
                                  const QString& object_name,
                                  LibraryGlyph glyph,
                                  int id) {
        auto* button = new QPushButton{label, tab_bar};
        button->setObjectName(object_name);
        button->setCheckable(true);
        button->setFixedHeight(22);
        button->setCursor(Qt::PointingHandCursor);
        button->setIcon(make_library_icon(glyph, QColor{QStringLiteral("#cfd3dc")}, 12));
        button->setIconSize(QSize{12, 12});
        tab_group_->addButton(button, id);
        tab_layout->addWidget(button);
        return button;
    };

    assets_tab_button_ = make_tab_btn(
        QStringLiteral("Assets (0)"), QStringLiteral("library-tab-assets"), LibraryGlyph::assets_tab, 0);
    songs_tab_button_ = make_tab_btn(
        QStringLiteral("Songs (0)"), QStringLiteral("library-tab-songs"), LibraryGlyph::songs_tab, 1);
    sermons_tab_button_ = make_tab_btn(
        QStringLiteral("Sermons (0)"), QStringLiteral("library-tab-sermons"), LibraryGlyph::sermons_tab, 2);
    assets_tab_button_->setChecked(true);
    QObject::connect(tab_group_, &QButtonGroup::idClicked, this, [this](int id) {
        if (id == 0) {
            show_assets();
        } else if (id == 1) {
            show_songs();
        } else if (id == 2) {
            show_sermons();
        }
    });
    toolbar_layout->addWidget(tab_bar);

    search_ = new QLineEdit{toolbar};
    search_->setObjectName(QStringLiteral("library-search"));
    search_->setPlaceholderText(QStringLiteral("Search Assets, Songs, or Sermons..."));
    search_->setClearButtonEnabled(true);
    search_->setFixedHeight(26);
    search_->setMinimumWidth(140);
    search_->setMaximumWidth(230);
    search_action_ = search_->addAction(
        make_library_icon(LibraryGlyph::search, ThemeManager::instance().palette().icon_muted, 12),
        QLineEdit::LeadingPosition);
    toolbar_layout->addWidget(search_);

    asset_filter_bar_ = new QWidget{toolbar};
    asset_filter_bar_->setObjectName(QStringLiteral("asset-filter-bar"));
    asset_filter_bar_->setFixedHeight(26);
    auto* asset_filter_layout = new QHBoxLayout(asset_filter_bar_);
    asset_filter_layout->setContentsMargins(2, 2, 2, 2);
    asset_filter_layout->setSpacing(2);
    asset_filter_group_ = new QButtonGroup{this};
    asset_filter_group_->setExclusive(true);

    const auto make_filter_btn = [&](const QString& label, const QString& object_name, int id) {
        auto* button = new QPushButton{label, asset_filter_bar_};
        button->setObjectName(object_name);
        button->setCheckable(true);
        button->setFixedHeight(22);
        button->setCursor(Qt::PointingHandCursor);
        asset_filter_group_->addButton(button, id);
        asset_filter_layout->addWidget(button);
        return button;
    };
    filter_all_button_ = make_filter_btn(QStringLiteral("All"), QStringLiteral("filter-assets-all"), 0);
    filter_images_button_ =
        make_filter_btn(QStringLiteral("Images"), QStringLiteral("filter-assets-images"), 1);
    filter_videos_button_ =
        make_filter_btn(QStringLiteral("Videos"), QStringLiteral("filter-assets-videos"), 2);
    filter_all_button_->setChecked(true);
    QObject::connect(asset_filter_group_, &QButtonGroup::idClicked, this, [this](int id) {
        if (id == 0) {
            set_asset_kind_filter(std::nullopt);
        } else if (id == 1) {
            set_asset_kind_filter(domain::AssetKind::image);
        } else if (id == 2) {
            set_asset_kind_filter(domain::AssetKind::video);
        }
    });
    toolbar_layout->addWidget(asset_filter_bar_);

    const auto add_action_btn = [&](QWidget* group_parent,
                                    QHBoxLayout* group_layout,
                                    const QString& label,
                                    const QString& object_name,
                                    const QString& tooltip,
                                    LibraryGlyph glyph,
                                    const QColor& icon_color,
                                    auto handler) {
        auto* button = new QPushButton{label, group_parent};
        button->setObjectName(object_name);
        button->setProperty("libAction", true);
        button->setProperty("primaryAction", false);
        button->setFixedHeight(26);
        button->setCursor(Qt::PointingHandCursor);
        button->setToolTip(tooltip);
        button->setIcon(make_library_icon(glyph, icon_color, 12));
        button->setIconSize(QSize{12, 12});
        QObject::connect(button, &QPushButton::clicked, this, handler);
        group_layout->addWidget(button);
        return button;
    };

    const auto make_action_group = [&] {
        auto* group = new QWidget{toolbar};
        group->setStyleSheet(QStringLiteral("background: transparent; border: none;"));
        auto* row = new QHBoxLayout(group);
        row->setContentsMargins(0, 0, 0, 0);
        row->setSpacing(4);
        toolbar_layout->addWidget(group);
        return std::pair{group, row};
    };

    const QColor neutral_icon{QStringLiteral("#cfd3dc")};
    auto [asset_group, asset_group_layout] = make_action_group();
    import_asset_button_ = add_action_btn(asset_group,
                                          asset_group_layout,
                                          QStringLiteral("Import Asset"),
                                          QStringLiteral("import-asset"),
                                          QStringLiteral("Import image or video background"),
                                          LibraryGlyph::import_doc,
                                          neutral_icon,
                                          [this] {
                                              if (import_asset_handler_) {
                                                  import_asset_handler_();
                                              }
                                          });

    auto [song_group, song_group_layout] = make_action_group();
    new_song_button_ = add_action_btn(song_group,
                                      song_group_layout,
                                      QStringLiteral("New Song"),
                                      QStringLiteral("new-song"),
                                      QStringLiteral("Create a new Song in Reflow editor"),
                                      LibraryGlyph::plus,
                                      neutral_icon,
                                      [this] {
                                          if (new_song_handler_) {
                                              new_song_handler_();
                                          }
                                      });
    import_song_button_ = add_action_btn(song_group,
                                         song_group_layout,
                                         QStringLiteral("Import Text"),
                                         QStringLiteral("import-song-text"),
                                         QStringLiteral("Import Song from text file"),
                                         LibraryGlyph::import_doc,
                                         neutral_icon,
                                         [this] {
                                             if (import_song_handler_) {
                                                 import_song_handler_();
                                             }
                                         });
    edit_song_button_ = add_action_btn(song_group,
                                       song_group_layout,
                                       QStringLiteral("Edit"),
                                       QStringLiteral("edit-song"),
                                       QStringLiteral("Edit selected Song"),
                                       LibraryGlyph::edit,
                                       neutral_icon,
                                       [this] {
                                           if (edit_song_handler_) {
                                               edit_song_row(songs_->currentRow());
                                           }
                                       });

    auto [sermon_group, sermon_group_layout] = make_action_group();
    new_sermon_button_ = add_action_btn(sermon_group,
                                        sermon_group_layout,
                                        QStringLiteral("New Sermon"),
                                        QStringLiteral("new-sermon"),
                                        QStringLiteral("Create a new Sermon slide deck"),
                                        LibraryGlyph::plus,
                                        neutral_icon,
                                        [this] {
                                            if (new_sermon_handler_) {
                                                new_sermon_handler_();
                                            }
                                        });
    import_sermon_button_ = add_action_btn(sermon_group,
                                           sermon_group_layout,
                                           QStringLiteral("Import Deck"),
                                           QStringLiteral("import-sermon-deck"),
                                           QStringLiteral("Import Sermon slide media"),
                                           LibraryGlyph::import_doc,
                                           neutral_icon,
                                           [this] {
                                               if (import_sermon_handler_) {
                                                   import_sermon_handler_();
                                               }
                                           });
    edit_sermon_button_ = add_action_btn(sermon_group,
                                         sermon_group_layout,
                                         QStringLiteral("Edit"),
                                         QStringLiteral("edit-sermon"),
                                         QStringLiteral("Edit selected Sermon deck"),
                                         LibraryGlyph::edit,
                                         neutral_icon,
                                         [this] {
                                             if (edit_sermon_handler_) {
                                                 edit_sermon_row(sermons_->currentRow());
                                             }
                                         });

    container_layout->addWidget(toolbar);

    // 4. Stacked content views + single shared lightweight empty-state overlay
    stack_ = new QStackedWidget{container};
    stack_->setObjectName(QStringLiteral("library-stack"));

    assets_ = new AssetListWidget{stack_};
    assets_->setObjectName(QStringLiteral("assets-list"));
    assets_->setViewMode(QListView::IconMode);
    assets_->setFlow(QListView::LeftToRight);
    assets_->setWrapping(true);
    assets_->setResizeMode(QListView::Adjust);
    assets_->setMovement(QListView::Static);
    assets_->setSelectionMode(QAbstractItemView::SingleSelection);
    assets_->setDragEnabled(true);
    assets_->setDragDropMode(QAbstractItemView::DragOnly);
    assets_->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    assets_->setMouseTracking(true);
    assets_->setSpacing(4);
    assets_->setIconSize(QSize{kThumbnailWidth, kThumbnailHeight});
    assets_->setGridSize(QSize{kAssetGridWidth, kAssetGridHeight});
    assets_->setUniformItemSizes(true);
    assets_->setItemDelegate(new AssetCardDelegate{assets_});

    auto* songs_list = new ContentListWidget{domain::ContentKind::song, stack_};
    auto* sermons_list = new ContentListWidget{domain::ContentKind::sermon, stack_};
    songs_ = songs_list;
    sermons_ = sermons_list;
    songs_->setObjectName(QStringLiteral("songs-list"));
    sermons_->setObjectName(QStringLiteral("sermons-list"));

    for (auto* list : {songs_, sermons_}) {
        list->setViewMode(QListView::ListMode);
        list->setFlow(QListView::LeftToRight);
        list->setWrapping(true);
        list->setResizeMode(QListView::Adjust);
        list->setMovement(QListView::Static);
        list->setSelectionMode(QAbstractItemView::SingleSelection);
        list->setDragEnabled(true);
        list->setDragDropMode(QAbstractItemView::DragOnly);
        list->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
        list->setMouseTracking(true);
        list->setSpacing(4);
        list->setGridSize(QSize{kContentGridWidth, kContentGridHeight});
        list->setUniformItemSizes(true);
        list->setItemDelegate(new ContentCardDelegate{list});
    }

    songs_list->set_action_handlers(
        [this](QListWidgetItem* item) {
            if (item != nullptr && add_content_handler_) {
                add_content_handler_(
                    domain::ContentRef::song(item->data(kContentIdRole).toString().toStdString()));
            }
        },
        [this](QListWidgetItem* item) {
            if (item != nullptr && edit_song_handler_) {
                edit_song_handler_(domain::SongId{item->data(kContentIdRole).toString().toStdString()});
            }
        });

    sermons_list->set_action_handlers(
        [this](QListWidgetItem* item) {
            if (item != nullptr && add_content_handler_) {
                add_content_handler_(
                    domain::ContentRef::sermon(item->data(kContentIdRole).toString().toStdString()));
            }
        },
        [this](QListWidgetItem* item) {
            if (item != nullptr && edit_sermon_handler_) {
                edit_sermon_handler_(
                    domain::SermonId{item->data(kContentIdRole).toString().toStdString()});
            }
        });

    songs_->setContextMenuPolicy(Qt::CustomContextMenu);
    QObject::connect(songs_, &QListWidget::customContextMenuRequested, this, [this](const QPoint& position) {
        auto* item = songs_->itemAt(position);
        if (item == nullptr) {
            return;
        }
        QMenu menu{this};
        if (add_content_handler_) {
            auto* add = menu.addAction(QStringLiteral("Add to Service Order"));
            QObject::connect(add, &QAction::triggered, this, [this, item] {
                if (add_content_handler_) {
                    add_content_handler_(
                        domain::ContentRef::song(item->data(kContentIdRole).toString().toStdString()));
                }
            });
        }
        if (edit_song_handler_) {
            auto* edit = menu.addAction(QStringLiteral("Edit Song"));
            QObject::connect(edit, &QAction::triggered, this, [this, item] {
                if (edit_song_handler_) {
                    edit_song_handler_(
                        domain::SongId{item->data(kContentIdRole).toString().toStdString()});
                }
            });
        }
        if (!menu.isEmpty()) {
            menu.exec(songs_->viewport()->mapToGlobal(position));
        }
    });

    QObject::connect(songs_, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem* item) {
        if (item != nullptr && add_content_handler_) {
            add_content_handler_(
                domain::ContentRef::song(item->data(kContentIdRole).toString().toStdString()));
        }
    });

    sermons_->setContextMenuPolicy(Qt::CustomContextMenu);
    QObject::connect(sermons_, &QListWidget::customContextMenuRequested, this, [this](const QPoint& position) {
        auto* item = sermons_->itemAt(position);
        if (item == nullptr) {
            return;
        }
        QMenu menu{this};
        if (add_content_handler_) {
            auto* add = menu.addAction(QStringLiteral("Add to Service Order"));
            QObject::connect(add, &QAction::triggered, this, [this, item] {
                if (add_content_handler_) {
                    add_content_handler_(
                        domain::ContentRef::sermon(item->data(kContentIdRole).toString().toStdString()));
                }
            });
        }
        if (edit_sermon_handler_) {
            auto* edit = menu.addAction(QStringLiteral("Edit Sermon"));
            QObject::connect(edit, &QAction::triggered, this, [this, item] {
                if (edit_sermon_handler_) {
                    edit_sermon_handler_(
                        domain::SermonId{item->data(kContentIdRole).toString().toStdString()});
                }
            });
        }
        if (!menu.isEmpty()) {
            menu.exec(sermons_->viewport()->mapToGlobal(position));
        }
    });

    QObject::connect(sermons_, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem* item) {
        if (item != nullptr && add_content_handler_) {
            add_content_handler_(
                domain::ContentRef::sermon(item->data(kContentIdRole).toString().toStdString()));
        }
    });

    QObject::connect(songs_, &QListWidget::currentRowChanged, this, [this](int) {
        update_action_states();
    });
    QObject::connect(sermons_, &QListWidget::currentRowChanged, this, [this](int) {
        update_action_states();
    });

    stack_->addWidget(assets_);
    stack_->addWidget(songs_);
    stack_->addWidget(sermons_);
    container_layout->addWidget(stack_, 1);

    // Single transparent empty-state overlay shared across all 3 views
    empty_overlay_ = new QWidget{stack_};
    empty_overlay_->setObjectName(QStringLiteral("library-empty-state"));
    empty_overlay_->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    auto* empty_layout = new QVBoxLayout(empty_overlay_);
    empty_layout->setContentsMargins(24, 12, 24, 12);
    empty_layout->setSpacing(4);
    empty_layout->addStretch(1);

    empty_icon_label_ = new QLabel{empty_overlay_};
    empty_icon_label_->setAlignment(Qt::AlignCenter);
    empty_layout->addWidget(empty_icon_label_);

    empty_title_label_ = new QLabel{empty_overlay_};
    empty_title_label_->setObjectName(QStringLiteral("library-empty-title"));
    empty_title_label_->setAlignment(Qt::AlignCenter);
    empty_layout->addWidget(empty_title_label_);

    empty_hint_label_ = new QLabel{empty_overlay_};
    empty_hint_label_->setObjectName(QStringLiteral("library-empty-hint"));
    empty_hint_label_->setAlignment(Qt::AlignCenter);
    empty_hint_label_->setWordWrap(true);
    empty_layout->addWidget(empty_hint_label_);
    empty_layout->addStretch(1);

    setWidget(container);

    auto* focus_search = new QShortcut{QKeySequence::Find, this};
    focus_search->setContext(Qt::WidgetWithChildrenShortcut);
    QObject::connect(focus_search, &QShortcut::activated, this, [this] {
        search_->setFocus(Qt::ShortcutFocusReason);
        search_->selectAll();
    });

    QObject::connect(search_, &QLineEdit::textChanged, this, [this] { apply_filter(); });

    set_tab(LibraryTab::assets, 0);
    update_tab_and_header_state();
    apply_theme();
    QObject::connect(&ThemeManager::instance(), &ThemeManager::theme_changed, this,
                     [this](ThemeId) { apply_theme(); });
}

void LibraryDockWidget::show_assets() { set_tab(LibraryTab::assets, 0); }

void LibraryDockWidget::show_songs() { set_tab(LibraryTab::songs, 1); }

void LibraryDockWidget::show_sermons() { set_tab(LibraryTab::sermons, 2); }

void LibraryDockWidget::set_asset_root(std::filesystem::path root) {
    if (asset_root_ != root) {
        thumbnail_cache_.clear();
    }
    asset_root_ = std::move(root);
}

void LibraryDockWidget::add_asset(domain::AssetRecord asset) {
    const auto filename = std::filesystem::path{asset.relative_path}.filename().string();
    auto* item = new QListWidgetItem{
        QString::fromStdString(filename.empty() ? asset.relative_path : filename)};
    item->setData(kContentIdRole, QString::fromStdString(asset.id.value));
    item->setData(kAssetKindRole, static_cast<int>(asset.kind));
    item->setData(kAssetSizeRole, static_cast<qulonglong>(asset.size_bytes));
    item->setData(kRelativePathRole, QString::fromStdString(asset.relative_path));
    item->setToolTip(QStringLiteral("%1\n%2 (%3 bytes)\nDrag onto a Song or Slide to set background")
                         .arg(QString::fromStdString(asset.relative_path))
                         .arg(format_compact_bytes(asset.size_bytes))
                         .arg(static_cast<qulonglong>(asset.size_bytes)));
    if (asset.kind == domain::AssetKind::image && !asset_root_.empty()) {
        const QIcon thumbnail = load_bounded_thumbnail(asset);
        if (!thumbnail.isNull()) {
            item->setIcon(thumbnail);
        }
    } else if (asset.kind == domain::AssetKind::video) {
        item->setIcon(shared_video_icon_);
    }
    assets_->addItem(item);
    apply_filter_to_asset(item, search_->text());
    update_tab_and_header_state();
    update_empty_state();
}

void LibraryDockWidget::add_song(domain::Song song) {
    const QString id = QString::fromStdString(song.id.value);
    QListWidgetItem* item = nullptr;
    for (int index = 0; index != songs_->count(); ++index) {
        if (songs_->item(index)->data(kContentIdRole).toString() == id) {
            item = songs_->item(index);
            break;
        }
    }
    if (item == nullptr) {
        item = new QListWidgetItem{};
        item->setData(kContentIdRole, id);
        songs_->addItem(item);
    }
    item->setText(QString::fromStdString(song.title));
    item->setData(kContentKindRole, static_cast<int>(domain::ContentKind::song));
    item->setData(kSlideCountRole, static_cast<int>(song.slides.size()));
    item->setData(kHasBgRole, song.default_background.has_value());
    item->setToolTip(QStringLiteral("%1\n%2 slide%3 · Double-click or drag to Service Order")
                         .arg(item->text())
                         .arg(song.slides.size())
                         .arg(song.slides.size() == 1 ? QString() : QStringLiteral("s")));
    apply_filter_to_content(item, search_->text());
    update_tab_and_header_state();
    update_empty_state();
}

void LibraryDockWidget::add_sermon(domain::SermonDeck deck) {
    const QString id = QString::fromStdString(deck.id.value);
    QListWidgetItem* item = nullptr;
    for (int index = 0; index != sermons_->count(); ++index) {
        if (sermons_->item(index)->data(kContentIdRole).toString() == id) {
            item = sermons_->item(index);
            break;
        }
    }
    if (item == nullptr) {
        item = new QListWidgetItem{};
        item->setData(kContentIdRole, id);
        sermons_->addItem(item);
    }
    item->setText(QString::fromStdString(deck.title));
    item->setData(kContentKindRole, static_cast<int>(domain::ContentKind::sermon));
    item->setData(kSlideCountRole, static_cast<int>(deck.slides.size()));
    item->setData(kHasBgRole, false);
    item->setToolTip(QStringLiteral("%1\n%2 slide%3 · Double-click or drag to Service Order")
                         .arg(item->text())
                         .arg(deck.slides.size())
                         .arg(deck.slides.size() == 1 ? QString() : QStringLiteral("s")));
    apply_filter_to_content(item, search_->text());
    update_tab_and_header_state();
    update_empty_state();
}

void LibraryDockWidget::clear_contents() {
    assets_->clear();
    songs_->clear();
    sermons_->clear();
    update_tab_and_header_state();
    update_action_states();
    update_empty_state();
}

void LibraryDockWidget::purge_non_live_caches() {
    thumbnail_cache_.clear();
}

void LibraryDockWidget::set_search_text(const QString& text) { search_->setText(text); }

void LibraryDockWidget::set_asset_kind_filter(std::optional<domain::AssetKind> kind) {
    asset_kind_filter_ = kind;
    if (!kind.has_value()) {
        filter_all_button_->setChecked(true);
    } else if (*kind == domain::AssetKind::image) {
        filter_images_button_->setChecked(true);
    } else {
        filter_videos_button_->setChecked(true);
    }
    apply_filter();
}

void LibraryDockWidget::activate_song_row(int row) {
    if (row >= 0 && row < songs_->count()) {
        const auto* item = songs_->item(row);
        if (add_content_handler_ && item != nullptr) {
            add_content_handler_(
                domain::ContentRef::song(item->data(kContentIdRole).toString().toStdString()));
        }
    }
}

void LibraryDockWidget::edit_song_row(int row) {
    if (row >= 0 && row < songs_->count()) {
        const auto* item = songs_->item(row);
        if (edit_song_handler_ && item != nullptr) {
            edit_song_handler_(domain::SongId{item->data(kContentIdRole).toString().toStdString()});
        }
    }
}

void LibraryDockWidget::activate_sermon_row(int row) {
    if (row >= 0 && row < sermons_->count()) {
        const auto* item = sermons_->item(row);
        if (add_content_handler_ && item != nullptr) {
            add_content_handler_(
                domain::ContentRef::sermon(item->data(kContentIdRole).toString().toStdString()));
        }
    }
}

void LibraryDockWidget::edit_sermon_row(int row) {
    if (row >= 0 && row < sermons_->count()) {
        const auto* item = sermons_->item(row);
        if (edit_sermon_handler_ && item != nullptr) {
            edit_sermon_handler_(
                domain::SermonId{item->data(kContentIdRole).toString().toStdString()});
        }
    }
}

int LibraryDockWidget::visible_asset_count() const noexcept {
    if (current_tab_ != LibraryTab::assets) {
        return 0;
    }
    int visible = 0;
    for (int index = 0; index != assets_->count(); ++index) {
        visible += assets_->item(index)->isHidden() ? 0 : 1;
    }
    return visible;
}

int LibraryDockWidget::visible_content_count() const noexcept {
    if (current_tab_ == LibraryTab::songs) {
        int visible = 0;
        for (int index = 0; index != songs_->count(); ++index) {
            visible += songs_->item(index)->isHidden() ? 0 : 1;
        }
        return visible;
    }
    if (current_tab_ == LibraryTab::sermons) {
        int visible = 0;
        for (int index = 0; index != sermons_->count(); ++index) {
            visible += sermons_->item(index)->isHidden() ? 0 : 1;
        }
        return visible;
    }
    return 0;
}

void LibraryDockWidget::resizeEvent(QResizeEvent* event) {
    QDockWidget::resizeEvent(event);
    layout_empty_overlay();
    update_responsive_grid_sizes();
}

void LibraryDockWidget::set_tab(LibraryTab tab, int index) {
    current_tab_ = tab;
    stack_->setCurrentIndex(index);
    if (tab == LibraryTab::assets) {
        assets_tab_button_->setChecked(true);
        search_->setPlaceholderText(QStringLiteral("Filter assets by filename..."));
    } else if (tab == LibraryTab::songs) {
        songs_tab_button_->setChecked(true);
        search_->setPlaceholderText(QStringLiteral("Filter songs by title..."));
    } else {
        sermons_tab_button_->setChecked(true);
        search_->setPlaceholderText(QStringLiteral("Filter sermons by title..."));
    }
    asset_filter_bar_->setVisible(tab == LibraryTab::assets);
    if (auto* toolbar = findChild<QWidget*>(QStringLiteral("library-toolbar")); toolbar != nullptr) {
        toolbar->updateGeometry();
    }
    update_action_states();
    update_empty_state();
    update_responsive_grid_sizes();
}

void LibraryDockWidget::apply_filter() {
    const auto query = search_->text();
    for (int index = 0; index != assets_->count(); ++index) {
        apply_filter_to_asset(assets_->item(index), query);
    }
    for (int index = 0; index != songs_->count(); ++index) {
        apply_filter_to_content(songs_->item(index), query);
    }
    for (int index = 0; index != sermons_->count(); ++index) {
        apply_filter_to_content(sermons_->item(index), query);
    }
    update_empty_state();
}

void LibraryDockWidget::apply_filter_to_asset(QListWidgetItem* item, const QString& query) const {
    if (item == nullptr) {
        return;
    }
    bool hidden = false;
    if (asset_kind_filter_.has_value()) {
        const auto kind = static_cast<domain::AssetKind>(item->data(kAssetKindRole).toInt());
        hidden = (kind != *asset_kind_filter_);
    }
    if (!hidden && !query.isEmpty()) {
        const bool matches_name = item->text().contains(query, Qt::CaseInsensitive);
        const bool matches_path =
            item->data(kRelativePathRole).toString().contains(query, Qt::CaseInsensitive);
        hidden = !matches_name && !matches_path;
    }
    item->setHidden(hidden);
}

void LibraryDockWidget::apply_filter_to_content(QListWidgetItem* item, const QString& query) const {
    if (item == nullptr) {
        return;
    }
    const bool hidden = !query.isEmpty() && !item->text().contains(query, Qt::CaseInsensitive);
    item->setHidden(hidden);
}

void LibraryDockWidget::update_tab_and_header_state() {
    const int asset_count = assets_->count();
    const int song_count = songs_->count();
    const int sermon_count = sermons_->count();
    const int total = asset_count + song_count + sermon_count;

    assets_tab_button_->setText(QStringLiteral("Assets (%1)").arg(asset_count));
    songs_tab_button_->setText(QStringLiteral("Songs (%1)").arg(song_count));
    sermons_tab_button_->setText(QStringLiteral("Sermons (%1)").arg(sermon_count));
    header_count_badge_->setText(
        total == 1 ? QStringLiteral("1 ITEM") : QStringLiteral("%1 ITEMS").arg(total));
}

void LibraryDockWidget::update_action_states() {
    const bool has_song_selected = songs_->currentRow() >= 0;
    const bool has_sermon_selected = sermons_->currentRow() >= 0;
    edit_song_button_->setEnabled(has_song_selected);
    edit_sermon_button_->setEnabled(has_sermon_selected);

    import_asset_button_->setProperty("primaryAction", current_tab_ == LibraryTab::assets);
    new_song_button_->setProperty("primaryAction", current_tab_ == LibraryTab::songs);
    new_sermon_button_->setProperty("primaryAction", current_tab_ == LibraryTab::sermons);
    refresh_widget_polish(import_asset_button_);
    refresh_widget_polish(new_song_button_);
    refresh_widget_polish(new_sermon_button_);

    const auto& p = ThemeManager::instance().palette();
    assets_tab_button_->setIcon(make_library_icon(
        LibraryGlyph::assets_tab,
        current_tab_ == LibraryTab::assets ? p.tab_active_fg : p.icon_neutral,
        12));
    songs_tab_button_->setIcon(make_library_icon(
        LibraryGlyph::songs_tab,
        current_tab_ == LibraryTab::songs ? p.tab_active_fg : p.icon_neutral,
        12));
    sermons_tab_button_->setIcon(make_library_icon(
        LibraryGlyph::sermons_tab,
        current_tab_ == LibraryTab::sermons ? p.tab_active_fg : p.icon_neutral,
        12));

    import_asset_button_->setIcon(make_library_icon(
        LibraryGlyph::import_doc,
        current_tab_ == LibraryTab::assets ? p.text_on_accent : p.icon_neutral,
        12));
    new_song_button_->setIcon(make_library_icon(
        LibraryGlyph::plus,
        current_tab_ == LibraryTab::songs ? p.text_on_accent : p.icon_neutral,
        12));
    import_song_button_->setIcon(make_library_icon(LibraryGlyph::import_doc, p.icon_neutral, 12));
    edit_song_button_->setIcon(make_library_icon(
        LibraryGlyph::edit, has_song_selected ? p.icon_neutral : p.text_disabled, 12));
    new_sermon_button_->setIcon(make_library_icon(
        LibraryGlyph::plus,
        current_tab_ == LibraryTab::sermons ? p.text_on_accent : p.icon_neutral,
        12));
    import_sermon_button_->setIcon(make_library_icon(LibraryGlyph::import_doc, p.icon_neutral, 12));
    edit_sermon_button_->setIcon(make_library_icon(
        LibraryGlyph::edit, has_sermon_selected ? p.icon_neutral : p.text_disabled, 12));
}

void LibraryDockWidget::update_empty_state() {
    if (empty_overlay_ == nullptr) {
        return;
    }
    int total_in_tab = 0;
    int visible_in_tab = 0;
    LibraryGlyph glyph = LibraryGlyph::assets_tab;
    QString title;
    QString hint;

    if (current_tab_ == LibraryTab::assets) {
        total_in_tab = assets_->count();
        visible_in_tab = visible_asset_count();
        glyph = LibraryGlyph::assets_tab;
        title = QStringLiteral("No Media Assets Yet");
        hint = QStringLiteral(
            "Click \"Import Asset\" to add background images or loop videos, then drag them onto "
            "Songs or Slides.");
    } else if (current_tab_ == LibraryTab::songs) {
        total_in_tab = songs_->count();
        visible_in_tab = visible_content_count();
        glyph = LibraryGlyph::songs_tab;
        title = QStringLiteral("Song Library Empty");
        hint = QStringLiteral(
            "Click \"New Song\" or \"Import Text\" to build reusable songs. Double-click or drag "
            "any song to the Service Order.");
    } else {
        total_in_tab = sermons_->count();
        visible_in_tab = visible_content_count();
        glyph = LibraryGlyph::sermons_tab;
        title = QStringLiteral("Sermon Library Empty");
        hint = QStringLiteral(
            "Click \"New Sermon\" or \"Import Deck\" to organize slide media for reuse across "
            "services.");
    }

    if (visible_in_tab > 0) {
        empty_overlay_->setVisible(false);
        return;
    }

    if (total_in_tab > 0) {
        title = QStringLiteral("No Matching Items");
        hint = QStringLiteral("No items match the current search or media filter.");
    }

    empty_icon_label_->setPixmap(
        make_library_icon(glyph, ThemeManager::instance().palette().icon_empty, 26).pixmap(26, 26));
    empty_title_label_->setText(title);
    empty_hint_label_->setText(hint);
    layout_empty_overlay();
    empty_overlay_->setVisible(true);
    empty_overlay_->raise();
}

void LibraryDockWidget::apply_theme() {
    const auto& p = ThemeManager::instance().palette();
    setStyleSheet(
        QStringLiteral(
            "#library-dock {"
            "  background-color: %1;"
            "  color: %2;"
            "}"
            "#library-dock-header {"
            "  background-color: %3;"
            "  border-top: 1px solid %4;"
            "  border-bottom: 1px solid %4;"
            "}"
            "#library-dock-title {"
            "  color: %5;"
            "  font-size: 10px;"
            "  font-weight: 700;"
            "  letter-spacing: 1.1px;"
            "  border: none;"
            "  background: transparent;"
            "}"
            "#library-total-count {"
            "  background-color: %6;"
            "  color: %7;"
            "  border: 1px solid %8;"
            "  border-radius: 3px;"
            "  padding: 0px 6px;"
            "  font-size: 9px;"
            "  font-weight: 700;"
            "}"
            "#library-dock-hint {"
            "  color: %9;"
            "  font-size: 10px;"
            "  border: none;"
            "  background: transparent;"
            "}"
            "#library-float-toggle {"
            "  background: transparent;"
            "  border: none;"
            "  border-radius: 3px;"
            "}"
            "#library-float-toggle:hover {"
            "  background-color: %10;"
            "}"
            "#library-toolbar {"
            "  background-color: %11;"
            "  border-bottom: 1px solid %12;"
            "}"
            "#library-tabs, #asset-filter-bar {"
            "  background-color: %13;"
            "  border: 1px solid %14;"
            "  border-radius: 4px;"
            "}"
            "#library-tabs QPushButton, #asset-filter-bar QPushButton {"
            "  background: transparent;"
            "  border: none;"
            "  border-radius: 3px;"
            "  color: %5;"
            "  font-size: 11px;"
            "  font-weight: 600;"
            "  padding: 2px 9px;"
            "}"
            "#library-tabs QPushButton:hover, #asset-filter-bar QPushButton:hover {"
            "  background-color: %15;"
            "  color: %2;"
            "}"
            "#library-tabs QPushButton:checked {"
            "  background-color: %16;"
            "  color: %17;"
            "}"
            "#asset-filter-bar QPushButton:checked {"
            "  background-color: %18;"
            "  color: %19;"
            "}"
            "#library-dock QLineEdit#library-search {"
            "  background-color: %13;"
            "  border: 1px solid %14;"
            "  border-radius: 4px;"
            "  color: %2;"
            "  font-size: 11px;"
            "  padding: 3px 8px;"
            "}"
            "#library-dock QLineEdit#library-search:focus {"
            "  border-color: %16;"
            "}"
            "#library-dock QPushButton[libAction=\"true\"] {"
            "  background-color: %20;"
            "  border: 1px solid %21;"
            "  border-radius: 4px;"
            "  color: %22;"
            "  font-size: 11px;"
            "  font-weight: 600;"
            "  padding: 3px 8px;"
            "}"
            "#library-dock QPushButton[libAction=\"true\"]:hover {"
            "  background-color: %23;"
            "  border-color: %24;"
            "}"
            "#library-dock QPushButton[libAction=\"true\"]:pressed {"
            "  background-color: %25;"
            "}"
            "#library-dock QPushButton[libAction=\"true\"]:disabled {"
            "  background-color: %26;"
            "  border-color: %27;"
            "  color: %28;"
            "}"
            "#library-dock QPushButton[libAction=\"true\"][primaryAction=\"true\"] {"
            "  background-color: %16;"
            "  border-color: %29;"
            "  color: %17;"
            "}"
            "#library-dock QPushButton[libAction=\"true\"][primaryAction=\"true\"]:hover {"
            "  background-color: %30;"
            "  border-color: %31;"
            "}"
            "#library-dock QPushButton[libAction=\"true\"][primaryAction=\"true\"]:pressed {"
            "  background-color: %32;"
            "}"
            "#library-dock QListWidget {"
            "  background-color: %1;"
            "  border: none;"
            "  color: %2;"
            "  outline: 0;"
            "  padding: 6px;"
            "}"
            "#library-dock QListWidget::item {"
            "  background: transparent;"
            "  border: none;"
            "  padding: 0px;"
            "}"
            "#library-empty-state { background: transparent; }"
            "#library-empty-title {"
            "  color: %22;"
            "  font-size: 12px;"
            "  font-weight: 600;"
            "  background: transparent;"
            "}"
            "#library-empty-hint {"
            "  color: %9;"
            "  font-size: 11px;"
            "  background: transparent;"
            "}")
            .arg(p.window_bg.name(),               // %1
                 p.text_primary.name(),           // %2
                 p.header_bg.name(),              // %3
                 p.border_strong.name(),          // %4
                 p.text_muted.name(),             // %5
                 p.badge_bg.name(),               // %6
                 p.badge_fg.name(),               // %7
                 p.badge_border.name(),           // %8
                 p.text_subtle.name(),            // %9
                 p.menu_hover_bg.name(),          // %10
                 p.toolbar_bg.name(),             // %11
                 p.border_mid.name(),             // %12
                 p.input_bg.name(),               // %13
                 p.border_subtle.name(),          // %14
                 p.list_hover_bg.name(),          // %15
                 p.accent.name(),                 // %16
                 p.text_on_accent.name(),         // %17
                 p.button_hover_bg.name(),        // %18
                 p.text_primary.name(),           // %19
                 p.button_alt_bg.name(),          // %20
                 p.button_alt_border.name(),      // %21
                 p.text_secondary.name(),         // %22
                 p.button_hover_bg.name(),        // %23
                 p.button_hover_border.name(),    // %24
                 p.button_pressed_bg.name(),      // %25
                 p.button_disabled_bg.name(),     // %26
                 p.button_disabled_border.name(), // %27
                 p.text_disabled.name(),          // %28
                 p.accent_border.name(),          // %29
                 p.accent_hover.name(),           // %30
                 p.accent_hover_border.name(),    // %31
                 p.accent_pressed.name()));       // %32

    shared_video_icon_ = make_library_icon(
        LibraryGlyph::video_media, QColor{QStringLiteral("#a78bfa")}, 32);
    for (int i = 0; i < assets_->count(); ++i) {
        auto* item = assets_->item(i);
        if (item != nullptr &&
            static_cast<domain::AssetKind>(item->data(kAssetKindRole).toInt()) ==
                domain::AssetKind::video) {
            item->setIcon(shared_video_icon_);
        }
    }
    if (float_toggle_button_ != nullptr) {
        float_toggle_button_->setIcon(
            make_library_icon(LibraryGlyph::float_dock, p.icon_muted, 12));
    }
    if (search_action_ != nullptr) {
        search_action_->setIcon(make_library_icon(LibraryGlyph::search, p.icon_muted, 12));
    }
    update_action_states();
    update_empty_state();
    assets_->viewport()->update();
    songs_->viewport()->update();
    sermons_->viewport()->update();
}

void LibraryDockWidget::layout_empty_overlay() {
    if (empty_overlay_ != nullptr && stack_ != nullptr) {
        empty_overlay_->setGeometry(stack_->rect());
    }
}

void LibraryDockWidget::update_responsive_grid_sizes() {
    const auto adapt_grid = [](QListWidget* list, int default_w, int min_w, int height) {
        if (list == nullptr || list->viewport() == nullptr) {
            return;
        }
        const int vw = list->viewport()->width();
        if (vw <= 0) {
            return;
        }
        const int target_w = vw < default_w + 16 ? std::max(min_w, vw - 16) : default_w;
        const QSize target{target_w, height};
        if (list->gridSize() != target) {
            list->setGridSize(target);
        }
    };
    adapt_grid(assets_, kAssetGridWidth, 128, kAssetGridHeight);
    adapt_grid(songs_, kContentGridWidth, 148, kContentGridHeight);
    adapt_grid(sermons_, kContentGridWidth, 148, kContentGridHeight);
}

QIcon LibraryDockWidget::load_bounded_thumbnail(const domain::AssetRecord& asset) {
    const QString cache_key = QStringLiteral("%1|%2|%3|%4|%5")
                                  .arg(QString::fromStdString(asset_root_.string()))
                                  .arg(QString::fromStdString(asset.id.value))
                                  .arg(QString::fromStdString(asset.sha256))
                                  .arg(QString::fromStdString(asset.relative_path))
                                  .arg(static_cast<qulonglong>(asset.size_bytes));
    if (const QIcon* cached = thumbnail_cache_.object(cache_key); cached != nullptr) {
        return *cached;
    }

    QImageReader reader{QString::fromStdString((asset_root_ / asset.relative_path).string())};
    reader.setAutoTransform(true);
    const auto source_size = reader.size();
    if (source_size.isValid()) {
        reader.setScaledSize(
            source_size.scaled(kThumbnailWidth, kThumbnailHeight, Qt::KeepAspectRatio));
    }
    QImage image = reader.read();
    if (image.isNull()) {
        return QIcon{};
    }
    if (image.width() > kThumbnailWidth || image.height() > kThumbnailHeight) {
        image = image.scaled(
            kThumbnailWidth, kThumbnailHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
    QPixmap pixmap = QPixmap::fromImage(image);
    if (pixmap.isNull()) {
        return QIcon{};
    }
    const int cost_kb = std::max(1, (pixmap.width() * pixmap.height() * 4) / 1024);
    auto* cached_icon = new QIcon{std::move(pixmap)};
    QIcon result = *cached_icon;
    thumbnail_cache_.insert(cache_key, cached_icon, cost_kb);
    return result;
}

}  // namespace shekinah::ui
