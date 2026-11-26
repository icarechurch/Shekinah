#include "shekinah/ui/service_order_widget.hpp"

#include "shekinah/ui/support/elided_label.hpp"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

namespace shekinah::ui {

void ServiceOrderWidget::setup_header(QVBoxLayout* layout) {
    auto* header_bar = new QWidget{this};
    header_bar->setObjectName(QStringLiteral("service-order-header"));
    header_bar->setFixedHeight(26);
    auto* header_layout = new QHBoxLayout(header_bar);
    header_layout->setContentsMargins(10, 0, 6, 0);
    header_layout->setSpacing(6);

    auto* title_label = new ElidedLabel{QStringLiteral("SERVICE ORDER"), header_bar};
    title_label->setObjectName(QStringLiteral("service-order-title"));
    header_layout->addWidget(title_label);
    header_layout->addStretch(1);

    count_badge_ = new QLabel{QStringLiteral("0 ITEMS"), header_bar};
    count_badge_->setObjectName(QStringLiteral("service-order-count"));
    count_badge_->setFixedHeight(18);
    count_badge_->setAlignment(Qt::AlignCenter);
    header_layout->addWidget(count_badge_, 0, Qt::AlignVCenter);

    add_button_ = new QPushButton{header_bar};
    add_button_->setObjectName(QStringLiteral("service-order-add"));
    add_button_->setFixedSize(20, 20);
    add_button_->setCursor(Qt::PointingHandCursor);
    add_button_->setToolTip(QStringLiteral("New or Import Presentation Item"));
    add_button_->setIconSize(QSize{13, 13});
    add_button_->setEnabled(false);
    QObject::connect(add_button_, &QPushButton::clicked, this, [this] { open_add_menu(); });
    header_layout->addWidget(add_button_);
    layout->addWidget(header_bar);
}

}  // namespace shekinah::ui
