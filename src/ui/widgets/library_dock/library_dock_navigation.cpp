#include "shekinah/ui/library_dock_widget.hpp"

#include <QLineEdit>
#include <QPushButton>

namespace shekinah::ui {

void LibraryDockWidget::show_assets() { set_tab(LibraryTab::assets, 0); }

void LibraryDockWidget::show_songs() { set_tab(LibraryTab::songs, 1); }

void LibraryDockWidget::show_sermons() { set_tab(LibraryTab::sermons, 2); }

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

}  // namespace shekinah::ui
