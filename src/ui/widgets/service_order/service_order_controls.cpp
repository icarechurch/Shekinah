#include "shekinah/ui/service_order_widget.hpp"

#include <QHBoxLayout>
#include <QPushButton>
#include <QVBoxLayout>

namespace shekinah::ui {

void ServiceOrderWidget::setup_controls(QVBoxLayout* layout) {
    auto* controls_bar = new QWidget{this};
    controls_bar->setObjectName(QStringLiteral("service-order-controls"));
    controls_bar->setFixedHeight(32);
    auto* controls = new QHBoxLayout(controls_bar);
    controls->setContentsMargins(6, 4, 6, 4);
    controls->setSpacing(4);

    move_up_button_ = new QPushButton{controls_bar};
    move_up_button_->setObjectName(QStringLiteral("service-order-move-up"));
    move_up_button_->setProperty("orderTool", true);
    move_up_button_->setFixedSize(28, 24);
    move_up_button_->setCursor(Qt::PointingHandCursor);
    move_up_button_->setToolTip(QStringLiteral("Move Item Up (Alt+Up)"));
    move_up_button_->setIconSize(QSize{13, 13});

    move_down_button_ = new QPushButton{controls_bar};
    move_down_button_->setObjectName(QStringLiteral("service-order-move-down"));
    move_down_button_->setProperty("orderTool", true);
    move_down_button_->setFixedSize(28, 24);
    move_down_button_->setCursor(Qt::PointingHandCursor);
    move_down_button_->setToolTip(QStringLiteral("Move Item Down (Alt+Down)"));
    move_down_button_->setIconSize(QSize{13, 13});

    edit_button_ = new QPushButton{controls_bar};
    edit_button_->setObjectName(QStringLiteral("service-order-edit"));
    edit_button_->setProperty("orderTool", true);
    edit_button_->setFixedSize(28, 24);
    edit_button_->setCursor(Qt::PointingHandCursor);
    edit_button_->setToolTip(QStringLiteral("Edit Selected Song or Sermon (Ctrl+Enter)"));
    edit_button_->setIconSize(QSize{13, 13});

    remove_button_ = new QPushButton{QStringLiteral("Remove"), controls_bar};
    remove_button_->setObjectName(QStringLiteral("service-order-remove"));
    remove_button_->setProperty("orderTool", true);
    remove_button_->setFixedHeight(24);
    remove_button_->setCursor(Qt::PointingHandCursor);
    remove_button_->setToolTip(QStringLiteral("Remove Selected Item from Service Order (Del)"));
    remove_button_->setIconSize(QSize{12, 12});

    QObject::connect(move_up_button_, &QPushButton::clicked, this, [this] { move_selected(-1); });
    QObject::connect(move_down_button_, &QPushButton::clicked, this, [this] { move_selected(1); });
    QObject::connect(edit_button_, &QPushButton::clicked, this, [this] { edit_selected(); });
    QObject::connect(remove_button_, &QPushButton::clicked, this, [this] { remove_selected(); });

    controls->addWidget(move_up_button_);
    controls->addWidget(move_down_button_);
    controls->addWidget(edit_button_);
    controls->addStretch(1);
    controls->addWidget(remove_button_);
    layout->addWidget(controls_bar);
}

}  // namespace shekinah::ui
