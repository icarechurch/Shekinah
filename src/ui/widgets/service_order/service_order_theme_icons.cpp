#include "shekinah/ui/service_order_widget.hpp"

#include "service_order_icons.hpp"
#include "shekinah/ui/support/theme.hpp"

#include <QIcon>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>

namespace shekinah::ui {
using namespace service_order_internal;

void ServiceOrderWidget::apply_theme_icons() {
    const auto& p = ThemeManager::instance().palette();
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

}  // namespace shekinah::ui
