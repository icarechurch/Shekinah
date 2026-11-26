#include "shekinah/ui/library_dock_widget.hpp"

#include "library_components.hpp"

#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QStackedWidget>
#include <QWidget>

namespace shekinah::ui {
using namespace library_internal;

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

}  // namespace shekinah::ui
