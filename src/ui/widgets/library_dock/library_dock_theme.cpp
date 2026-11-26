#include "shekinah/ui/library_dock_widget.hpp"

#include "library_components.hpp"
#include "library_dock_theme_styles.hpp"

#include "shekinah/ui/support/theme.hpp"

#include <QColor>
#include <QIcon>
#include <QListWidget>
#include <QPushButton>
#include <QSize>
#include <QStackedWidget>

#include <algorithm>

namespace shekinah::ui {
using namespace library_internal;

void LibraryDockWidget::apply_theme() {
    const auto& p = ThemeManager::instance().palette();
    setStyleSheet(build_library_dock_stylesheet(p));

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

}  // namespace shekinah::ui
