#include "shekinah/ui/library_dock_widget.hpp"

#include "library_components.hpp"

#include "shekinah/ui/support/theme.hpp"

#include <QLabel>
#include <QListWidget>
#include <QWidget>

namespace shekinah::ui {
using namespace library_internal;

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
