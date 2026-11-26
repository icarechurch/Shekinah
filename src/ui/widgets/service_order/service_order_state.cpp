#include "shekinah/ui/service_order_widget.hpp"

#include "service_order_primitives.hpp"

#include <QLineEdit>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>

#include <utility>

namespace shekinah::ui {

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

}  // namespace shekinah::ui
