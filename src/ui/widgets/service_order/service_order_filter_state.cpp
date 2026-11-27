#include "shekinah/ui/service_order_widget.hpp"

#include <QLabel>
#include <QLineEdit>
#include <QListWidget>

namespace shekinah::ui {

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

}  // namespace shekinah::ui
