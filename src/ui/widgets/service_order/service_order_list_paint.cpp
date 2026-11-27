#include "service_order_list.hpp"

#include "shekinah/ui/support/theme.hpp"

#include <QPainter>

namespace shekinah::ui::service_order_internal {

void ServiceOrderListWidget::paintEvent(QPaintEvent* event) {
    QListWidget::paintEvent(event);
    if (content_drop_active_) {
        QPainter p{viewport()};
        p.setRenderHint(QPainter::Antialiasing, true);
        p.setPen(QPen{ThemeManager::instance().palette().accent, 2.0});
        p.drawRoundedRect(viewport()->rect().adjusted(2, 2, -2, -2), 4.0, 4.0);
    }
}

}  // namespace shekinah::ui::service_order_internal
