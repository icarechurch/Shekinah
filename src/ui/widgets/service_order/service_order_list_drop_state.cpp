#include "service_order_list.hpp"
#include "service_order_roles.hpp"

#include "shekinah/ui/support/asset_drag.hpp"
#include "shekinah/ui/support/content_drag.hpp"

#include <QMimeData>

namespace shekinah::ui::service_order_internal {

void ServiceOrderListWidget::set_drop_hover_state(const QMimeData* data, QPoint pos) {
    clear_drop_hover_state();
    if (asset_id_from_mime_data(data).has_value()) {
        if (auto* target = itemAt(pos); target != nullptr) {
            target->setData(kDropTargetRole, true);
            hovered_drop_row_ = row(target);
        }
    } else if (content_ref_from_mime_data(data).has_value()) {
        content_drop_active_ = true;
    }
    viewport()->update();
}

void ServiceOrderListWidget::clear_drop_hover_state() {
    if (hovered_drop_row_ >= 0 && hovered_drop_row_ < count()) {
        item(hovered_drop_row_)->setData(kDropTargetRole, false);
    }
    hovered_drop_row_ = -1;
    content_drop_active_ = false;
    viewport()->update();
}

bool ServiceOrderListWidget::handle_drop(const QMimeData* data, QPoint position) {
    clear_drop_hover_state();
    if (asset_id_from_mime_data(data).has_value()) {
        return asset_drop_handler_ && asset_drop_handler_(data, position);
    }
    if (content_ref_from_mime_data(data).has_value()) {
        return content_drop_handler_ && content_drop_handler_(data);
    }
    return false;
}

bool ServiceOrderListWidget::accepts_drop(const QMimeData* data) const {
    return asset_id_from_mime_data(data).has_value() ||
           content_ref_from_mime_data(data).has_value();
}

}  // namespace shekinah::ui::service_order_internal
