#include "shekinah/ui/library_dock_widget.hpp"

#include "library_components.hpp"

#include "shekinah/ui/support/theme.hpp"

#include <QButtonGroup>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>

namespace shekinah::ui {
using namespace library_internal;

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

}  // namespace shekinah::ui
