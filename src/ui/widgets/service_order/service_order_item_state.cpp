#include "shekinah/ui/service_order_widget.hpp"

#include "service_order_roles.hpp"

#include <QLabel>
#include <QListWidget>

#include <utility>

namespace shekinah::ui {
using namespace service_order_internal;

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

}  // namespace shekinah::ui
