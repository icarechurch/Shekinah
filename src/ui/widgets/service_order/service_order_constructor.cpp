#include "shekinah/ui/service_order_widget.hpp"

#include "service_order_list.hpp"

#include "shekinah/ui/support/asset_drag.hpp"
#include "shekinah/ui/support/content_drag.hpp"
#include "shekinah/ui/support/elided_label.hpp"
#include "shekinah/ui/support/theme.hpp"

#include <QAbstractItemView>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>

namespace shekinah::ui {
ServiceOrderWidget::ServiceOrderWidget(QWidget* parent) : QWidget(parent) {
    setObjectName(QStringLiteral("service-order"));

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    setup_header(layout);
    setup_list(layout);
    setup_controls(layout);
    setup_filter(layout);

    apply_theme();
    QObject::connect(&ThemeManager::instance(), &ThemeManager::theme_changed, this,
                     [this](ThemeId) { apply_theme(); });

    update_empty_state();
    update_controls_state();
}

}  // namespace shekinah::ui
