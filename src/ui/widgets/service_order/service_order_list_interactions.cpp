#include "shekinah/ui/service_order_widget.hpp"

#include "service_order_list.hpp"

#include "shekinah/ui/support/asset_drag.hpp"
#include "shekinah/ui/support/content_drag.hpp"

#include <QListWidget>
#include <QMimeData>

namespace shekinah::ui {

void ServiceOrderWidget::connect_list_interactions() {
    auto* list_widget = static_cast<service_order_internal::ServiceOrderListWidget*>(list_);
    list_widget->set_viewport_resized_handler([this] {
        if (empty_overlay_ != nullptr && list_ != nullptr) {
            empty_overlay_->setGeometry(list_->viewport()->rect());
        }
    });
    list_widget->set_asset_drop_handler([this](const QMimeData* data, QPoint position) {
        const auto asset = asset_id_from_mime_data(data);
        auto* item = list_->itemAt(position);
        const auto row = item == nullptr ? -1 : list_->row(item);
        if (!asset.has_value() || row < 0 || row >= static_cast<int>(items_.size()) ||
            !asset_drop_handler_) {
            return false;
        }
        asset_drop_handler_(items_[static_cast<std::size_t>(row)].item.content, *asset);
        return true;
    });
    list_widget->set_content_drop_handler([this](const QMimeData* data) {
        const auto content = content_ref_from_mime_data(data);
        if (!content.has_value() || !content_drop_handler_) {
            return false;
        }
        content_drop_handler_(*content);
        return true;
    });
    list_widget->set_key_action_handlers(
        [this](int delta) { move_selected(delta); },
        [this] { remove_selected(); },
        [this] { edit_selected(); });

    QObject::connect(list_, &QListWidget::currentRowChanged, this, [this](int row) {
        update_controls_state();
        if (row >= 0 && row < static_cast<int>(items_.size()) && selection_handler_) {
            selection_handler_(items_[static_cast<std::size_t>(row)].item.id);
        }
    });
    QObject::connect(list_, &QListWidget::customContextMenuRequested, this,
                     [this](const QPoint& pos) { open_context_menu(pos); });
    QObject::connect(list_, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem*) {
        edit_selected();
    });
}

}  // namespace shekinah::ui
