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

}  // namespace shekinah::ui
