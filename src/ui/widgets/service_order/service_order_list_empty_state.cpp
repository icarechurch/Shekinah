#include "shekinah/ui/service_order_widget.hpp"

#include <QLabel>
#include <QListWidget>
#include <QVBoxLayout>

namespace shekinah::ui {

void ServiceOrderWidget::build_empty_overlay() {
    empty_overlay_ = new QWidget{list_->viewport()};
    empty_overlay_->setObjectName(QStringLiteral("service-order-empty-state"));
    empty_overlay_->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    auto* empty_layout = new QVBoxLayout(empty_overlay_);
    empty_layout->setContentsMargins(18, 24, 18, 24);
    empty_layout->setSpacing(6);
    empty_layout->addStretch(1);

    empty_icon_label_ = new QLabel{empty_overlay_};
    empty_icon_label_->setAlignment(Qt::AlignCenter);
    empty_layout->addWidget(empty_icon_label_);

    empty_title_label_ = new QLabel{QStringLiteral("Service Order Empty"), empty_overlay_};
    empty_title_label_->setObjectName(QStringLiteral("service-order-empty-title"));
    empty_title_label_->setAlignment(Qt::AlignCenter);
    empty_layout->addWidget(empty_title_label_);

    empty_hint_label_ = new QLabel{
        QStringLiteral("Drag a Song or Sermon here, or double-click an item in the Library below."),
        empty_overlay_};
    empty_hint_label_->setObjectName(QStringLiteral("service-order-empty-hint"));
    empty_hint_label_->setAlignment(Qt::AlignCenter);
    empty_hint_label_->setWordWrap(true);
    empty_layout->addWidget(empty_hint_label_);
    empty_layout->addStretch(1);
}

}  // namespace shekinah::ui
