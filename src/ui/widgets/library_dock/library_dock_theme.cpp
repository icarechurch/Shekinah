#include "shekinah/ui/library_dock_widget.hpp"
#include "library_components.hpp"
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
using namespace library_internal;
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
