#pragma once

#include "shekinah/ui/library_dock_widget.hpp"
#include "shekinah/ui/support/asset_drag.hpp"
#include "shekinah/ui/support/content_drag.hpp"
#include "shekinah/ui/support/theme.hpp"

#include <QAbstractItemDelegate>
#include <QColor>
#include <QIcon>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QRect>
#include <QStyle>
#include <QWidget>

#include <algorithm>
#include <cstdint>

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
constexpr int kThumbnailCacheCapKb = 4096;

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

QIcon make_library_icon(LibraryGlyph glyph, const QColor& color, int size = 14);
void draw_content_badge_glyph(QPainter& p,
                              const QRectF& rect,
                              domain::ContentKind kind,
                              const QColor& color);
QString format_compact_bytes(std::uint64_t bytes);
QRect quick_add_rect_for_card(const QRect& card_rect);

QAbstractItemDelegate* make_asset_card_delegate(QObject* parent);
QAbstractItemDelegate* make_content_card_delegate(QObject* parent);

void refresh_widget_polish(QWidget* widget);

}  // namespace library_internal
}  // namespace shekinah::ui
