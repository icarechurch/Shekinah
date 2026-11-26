#include "shekinah/ui/service_order_widget.hpp"
#include "service_order_components.hpp"
#include "shekinah/ui/support/asset_drag.hpp"
#include "shekinah/ui/support/content_drag.hpp"
#include "shekinah/ui/support/elided_label.hpp"
#include "shekinah/ui/support/theme.hpp"

#include <QAbstractItemView>
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QFrame>
#include <QHBoxLayout>
#include <QIcon>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMenu>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QPushButton>
#include <QResizeEvent>
#include <QStyledItemDelegate>
#include <QVBoxLayout>

#include <algorithm>

namespace shekinah::ui {
void ServiceOrderWidget::apply_theme() {
    const auto& p = ThemeManager::instance().palette();
    setStyleSheet(
        QStringLiteral(
            "#service-order {"
            "  background-color: %1;"
            "  border-right: 1px solid %2;"
            "  color: %3;"
            "}"
            "#service-order-header {"
            "  background-color: %4;"
            "  border-bottom: 1px solid %2;"
            "}"
            "#service-order-title {"
            "  color: %5;"
            "  font-size: 10px;"
            "  font-weight: 700;"
            "  letter-spacing: 1.1px;"
            "  border: none;"
            "  background: transparent;"
            "}"
            "#service-order-count {"
            "  background-color: %6;"
            "  color: %7;"
            "  border: 1px solid %8;"
            "  border-radius: 3px;"
            "  padding: 0px 6px;"
            "  font-size: 9px;"
            "  font-weight: 700;"
            "}"
            "#service-order-add {"
            "  background: transparent;"
            "  border: none;"
            "  border-radius: 3px;"
            "}"
            "#service-order-add:hover {"
            "  background-color: %9;"
            "}"
            "#service-order-add:disabled {"
            "  opacity: 0.4;"
            "}"
            "#service-order QListWidget#service-order-list {"
            "  background-color: %1;"
            "  border: none;"
            "  outline: 0;"
            "}"
            "#service-order-empty-state { background: transparent; }"
            "#service-order-empty-title {"
            "  color: %10;"
            "  font-size: 12px;"
            "  font-weight: 600;"
            "  background: transparent;"
            "}"
            "#service-order-empty-hint {"
            "  color: %11;"
            "  font-size: 11px;"
            "  background: transparent;"
            "}"
            "#service-order-controls {"
            "  background-color: %12;"
            "  border-top: 1px solid %13;"
            "}"
            "#service-order-filter-bar {"
            "  background-color: %14;"
            "  border-top: 1px solid %13;"
            "}"
            "#service-order QPushButton[orderTool=\"true\"] {"
            "  background-color: %15;"
            "  border: 1px solid %16;"
            "  border-radius: 4px;"
            "  color: %17;"
            "  font-size: 11px;"
            "  font-weight: 600;"
            "  padding: 2px 6px;"
            "}"
            "#service-order QPushButton[orderTool=\"true\"]:hover {"
            "  background-color: %18;"
            "  border-color: %19;"
            "}"
            "#service-order QPushButton[orderTool=\"true\"]:pressed {"
            "  background-color: %20;"
            "}"
            "#service-order QPushButton[orderTool=\"true\"]:disabled {"
            "  background-color: %21;"
            "  border-color: %22;"
            "  color: %23;"
            "}"
            "#service-order QPushButton#service-order-remove:hover:enabled {"
            "  background-color: %24;"
            "  border-color: %25;"
            "  color: %26;"
            "}"
            "#service-order QLineEdit#service-order-filter {"
            "  background-color: %27;"
            "  border: 1px solid %28;"
            "  border-radius: 4px;"
            "  color: %3;"
            "  font-size: 11px;"
            "  padding: 3px 8px;"
            "}"
            "#service-order QLineEdit#service-order-filter:focus {"
            "  border-color: %29;"
            "}")
            .arg(p.panel_bg.name(),               // %1
                 p.border_strong.name(),          // %2
                 p.text_primary.name(),           // %3
                 p.header_bg.name(),              // %4
                 p.text_muted.name(),             // %5
                 p.badge_bg.name(),               // %6
                 p.badge_fg.name(),               // %7
                 p.badge_border.name(),           // %8
                 p.menu_hover_bg.name(),          // %9
                 p.text_secondary.name(),         // %10
                 p.text_subtle.name(),            // %11
                 p.toolbar_bg.name(),             // %12
                 p.border_mid.name(),             // %13
                 p.filter_bar_bg.name(),          // %14
                 p.button_alt_bg.name(),          // %15
                 p.button_alt_border.name(),      // %16
                 p.text_secondary.name(),         // %17
                 p.button_hover_bg.name(),        // %18
                 p.button_hover_border.name(),    // %19
                 p.button_pressed_bg.name(),      // %20
                 p.button_disabled_bg.name(),     // %21
                 p.button_disabled_border.name(), // %22
                 p.text_disabled.name(),          // %23
                 p.danger_hover_bg.name(),        // %24
                 p.danger_hover_border.name(),    // %25
                 p.danger_text_on_bg.name(),      // %26
                 p.input_bg.name(),               // %27
                 p.border_subtle.name(),          // %28
                 p.accent.name()));               // %29

    add_button_->setIcon(make_order_icon(OrderGlyph::plus, p.icon_neutral, 13));
    empty_icon_label_->setPixmap(
        make_order_icon(OrderGlyph::empty_stack, p.icon_empty, 28).pixmap(28, 28));
    move_up_button_->setIcon(make_order_icon(OrderGlyph::chevron_up, p.icon_neutral, 13));
    move_down_button_->setIcon(make_order_icon(OrderGlyph::chevron_down, p.icon_neutral, 13));
    edit_button_->setIcon(make_order_icon(OrderGlyph::edit, p.icon_neutral, 13));
    remove_button_->setIcon(make_order_icon(OrderGlyph::trash, p.danger_icon, 12));
    if (list_ != nullptr && list_->viewport() != nullptr) {
        list_->viewport()->update();
    }
}

void ServiceOrderWidget::set_items(std::vector<ServiceOrderView> items) {
    items_ = std::move(items);
    list_->clear();
    for (std::size_t index = 0; index != items_.size(); ++index) {
        auto& view = items_[index];
        if (live_content_.has_value()) {
            view.is_live = (view.item.content == *live_content_);
        }
        auto* item = new QListWidgetItem{view.label};
        item->setData(kKindRole, static_cast<int>(view.item.content.kind));
        item->setData(kSlideCountRole, view.slide_count);
        item->setData(kHasBgRole, view.has_background);
        item->setData(kIsLiveRole, view.is_live);
        item->setData(kOrdinalRole, static_cast<int>(index + 1));
        item->setData(kDropTargetRole, false);
        const QString kind_name = view.item.content.kind == domain::ContentKind::song
                                      ? QStringLiteral("Song")
                                      : QStringLiteral("Sermon");
        QString tip = QStringLiteral("%1 (%2)").arg(view.label, kind_name);
        if (view.slide_count > 0) {
            tip += QStringLiteral("\n%1 slide%2")
                       .arg(view.slide_count)
                       .arg(view.slide_count == 1 ? QString() : QStringLiteral("s"));
        }
        item->setToolTip(tip);
        list_->addItem(item);
    }
    count_badge_->setText(
        QStringLiteral("%1 ITEM%2")
            .arg(static_cast<qulonglong>(items_.size()))
            .arg(items_.size() == 1 ? QString() : QStringLiteral("S")));
    apply_filter();
    update_controls_state();
}

void ServiceOrderWidget::update_item_slide_count(const domain::ContentRef& content, int slide_count) {
    for (int i = 0; i < list_->count() && i < static_cast<int>(items_.size()); ++i) {
        auto& view = items_[static_cast<std::size_t>(i)];
        if (view.item.content != content) {
            continue;
        }
        view.slide_count = slide_count;
        auto* item = list_->item(i);
        item->setData(kSlideCountRole, slide_count);
        const QString kind_name = view.item.content.kind == domain::ContentKind::song
                                      ? QStringLiteral("Song")
                                      : QStringLiteral("Sermon");
        QString tip = QStringLiteral("%1 (%2)").arg(view.label, kind_name);
        if (view.slide_count > 0) {
            tip += QStringLiteral("\n%1 slide%2")
                       .arg(view.slide_count)
                       .arg(view.slide_count == 1 ? QString() : QStringLiteral("s"));
        }
        item->setToolTip(tip);
    }
    list_->viewport()->update();
}

void ServiceOrderWidget::set_live_content(std::optional<domain::ContentRef> live_content) {
    live_content_ = std::move(live_content);
    for (int i = 0; i < list_->count() && i < static_cast<int>(items_.size()); ++i) {
        const bool live = live_content_.has_value() &&
                          items_[static_cast<std::size_t>(i)].item.content == *live_content_;
        items_[static_cast<std::size_t>(i)].is_live = live;
        list_->item(i)->setData(kIsLiveRole, live);
    }
    list_->viewport()->update();
}

void ServiceOrderWidget::set_authoring_handlers(std::function<void()> new_song,
                                                std::function<void()> import_song,
                                                std::function<void()> new_sermon,
                                                std::function<void()> import_sermon) {
    new_song_handler_ = std::move(new_song);
    import_song_handler_ = std::move(import_song);
    new_sermon_handler_ = std::move(new_sermon);
    import_sermon_handler_ = std::move(import_sermon);
    const bool any = static_cast<bool>(new_song_handler_) ||
                     static_cast<bool>(import_song_handler_) ||
                     static_cast<bool>(new_sermon_handler_) ||
                     static_cast<bool>(import_sermon_handler_);
    add_button_->setEnabled(any);
}

void ServiceOrderWidget::select_row(int row) {
    list_->setCurrentRow(row);
    update_controls_state();
}

int ServiceOrderWidget::count() const noexcept { return list_->count(); }

void ServiceOrderWidget::apply_filter() {
    const QString query = filter_input_->text().trimmed();
    for (int i = 0; i < list_->count() && i < static_cast<int>(items_.size()); ++i) {
        auto* item = list_->item(i);
        const auto kind_label = items_[static_cast<std::size_t>(i)].item.content.kind ==
                                        domain::ContentKind::song
                                    ? QStringLiteral("song")
                                    : QStringLiteral("sermon");
        const bool matches = query.isEmpty() ||
                             item->text().contains(query, Qt::CaseInsensitive) ||
                             kind_label.contains(query, Qt::CaseInsensitive);
        item->setHidden(!matches);
    }
    update_empty_state();
    update_controls_state();
}

void ServiceOrderWidget::update_empty_state() {
    if (empty_overlay_ == nullptr) {
        return;
    }
    empty_overlay_->setGeometry(list_->viewport()->rect());
    if (items_.empty()) {
        empty_title_label_->setText(QStringLiteral("Service Order Empty"));
        empty_hint_label_->setText(QStringLiteral(
            "Drag a Song or Sermon here, or double-click an item in the Library below."));
        empty_overlay_->setVisible(true);
        return;
    }
    bool any_visible = false;
    for (int i = 0; i < list_->count(); ++i) {
        if (!list_->item(i)->isHidden()) {
            any_visible = true;
            break;
        }
    }
    if (!any_visible) {
        empty_title_label_->setText(QStringLiteral("No Matching Items"));
        empty_hint_label_->setText(QStringLiteral("Clear the filter to see all service items."));
        empty_overlay_->setVisible(true);
    } else {
        empty_overlay_->setVisible(false);
    }
}

void ServiceOrderWidget::update_controls_state() {
    const int row = list_->currentRow();
    const bool valid = row >= 0 && row < static_cast<int>(items_.size());
    const bool filtering = !filter_input_->text().trimmed().isEmpty();
    move_up_button_->setEnabled(valid && !filtering && row > 0);
    move_down_button_->setEnabled(valid && !filtering && row + 1 < static_cast<int>(items_.size()));
    edit_button_->setEnabled(valid && static_cast<bool>(edit_handler_));
    remove_button_->setEnabled(valid);
}

void ServiceOrderWidget::move_selected(int delta) {
    const int row = list_->currentRow();
    const int target = row + delta;
    if (row >= 0 && row < static_cast<int>(items_.size()) &&
        target >= 0 && target < static_cast<int>(items_.size()) &&
        move_handler_) {
        move_handler_(items_[static_cast<std::size_t>(row)].item.id, target);
    }
}

void ServiceOrderWidget::remove_selected() {
    const int row = list_->currentRow();
    if (row >= 0 && row < static_cast<int>(items_.size()) && remove_handler_) {
        remove_handler_(items_[static_cast<std::size_t>(row)].item.id);
    }
}

void ServiceOrderWidget::edit_selected() {
    const int row = list_->currentRow();
    if (row >= 0 && row < static_cast<int>(items_.size()) && edit_handler_) {
        edit_handler_(items_[static_cast<std::size_t>(row)].item.content);
    }
}

void ServiceOrderWidget::open_add_menu() {
    QMenu menu{this};
    if (new_song_handler_) {
        auto* action = menu.addAction(QStringLiteral("New Song..."));
        QObject::connect(action, &QAction::triggered, this, [this] { new_song_handler_(); });
    }
    if (import_song_handler_) {
        auto* action = menu.addAction(QStringLiteral("Import Song Lyrics..."));
        QObject::connect(action, &QAction::triggered, this, [this] { import_song_handler_(); });
    }
    if ((new_song_handler_ || import_song_handler_) &&
        (new_sermon_handler_ || import_sermon_handler_)) {
        menu.addSeparator();
    }
    if (new_sermon_handler_) {
        auto* action = menu.addAction(QStringLiteral("New Sermon..."));
        QObject::connect(action, &QAction::triggered, this, [this] { new_sermon_handler_(); });
    }
    if (import_sermon_handler_) {
        auto* action = menu.addAction(QStringLiteral("Import Sermon Deck..."));
        QObject::connect(action, &QAction::triggered, this, [this] { import_sermon_handler_(); });
    }
    if (menu.isEmpty()) {
        return;
    }
    menu.exec(add_button_->mapToGlobal(QPoint{0, add_button_->height()}));
}

void ServiceOrderWidget::open_context_menu(const QPoint& viewport_pos) {
    auto* clicked = list_->itemAt(viewport_pos);
    if (clicked == nullptr) {
        return;
    }
    const int row = list_->row(clicked);
    if (row < 0 || row >= static_cast<int>(items_.size())) {
        return;
    }
    list_->setCurrentRow(row);

    QMenu menu{this};
    const bool filtering = !filter_input_->text().trimmed().isEmpty();
    auto* up_action = menu.addAction(QStringLiteral("Move Up"));
    up_action->setEnabled(!filtering && row > 0);
    QObject::connect(up_action, &QAction::triggered, this, [this] { move_selected(-1); });

    auto* down_action = menu.addAction(QStringLiteral("Move Down"));
    down_action->setEnabled(!filtering && row + 1 < static_cast<int>(items_.size()));
    QObject::connect(down_action, &QAction::triggered, this, [this] { move_selected(1); });

    if (edit_handler_) {
        menu.addSeparator();
        const bool is_song =
            items_[static_cast<std::size_t>(row)].item.content.kind == domain::ContentKind::song;
        auto* edit_action = menu.addAction(is_song ? QStringLiteral("Edit Song...")
                                                   : QStringLiteral("Edit Sermon..."));
        QObject::connect(edit_action, &QAction::triggered, this, [this] { edit_selected(); });
    }

    menu.addSeparator();
    auto* remove_action = menu.addAction(QStringLiteral("Remove from Service Order"));
    QObject::connect(remove_action, &QAction::triggered, this, [this] { remove_selected(); });

    menu.exec(list_->viewport()->mapToGlobal(viewport_pos));
}

}  // namespace shekinah::ui
