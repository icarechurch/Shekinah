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
