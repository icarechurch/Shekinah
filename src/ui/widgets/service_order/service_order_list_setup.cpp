#include "shekinah/ui/service_order_widget.hpp"

#include "service_order_list.hpp"

#include "shekinah/ui/support/asset_drag.hpp"
#include "shekinah/ui/support/content_drag.hpp"

#include <QAbstractItemView>
#include <QFrame>
#include <QLabel>
#include <QListWidget>
#include <QMimeData>
#include <QVBoxLayout>

namespace shekinah::ui {

void ServiceOrderWidget::setup_list(QVBoxLayout* layout) {
    auto* list_widget = new service_order_internal::ServiceOrderListWidget{this};
    list_ = list_widget;
    list_->setObjectName(QStringLiteral("service-order-list"));
    list_->setFrameShape(QFrame::NoFrame);
    list_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    list_->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    list_->setSelectionMode(QAbstractItemView::SingleSelection);
    list_->setUniformItemSizes(true);
    list_->setAcceptDrops(true);
    list_->viewport()->setAcceptDrops(true);
    list_->setContextMenuPolicy(Qt::CustomContextMenu);

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
    layout->addWidget(list_, 1);
}

}  // namespace shekinah::ui
