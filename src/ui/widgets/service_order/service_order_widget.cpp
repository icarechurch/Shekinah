#include "shekinah/ui/service_order_widget.hpp"
#include "shekinah/ui/support/asset_drag.hpp"
#include "shekinah/ui/support/content_drag.hpp"

#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QListWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>

namespace shekinah::ui {
namespace {

class ServiceOrderListWidget final : public QListWidget {
public:
    using AssetDropHandler = std::function<bool(const QMimeData*, QPoint)>;
    using ContentDropHandler = std::function<bool(const QMimeData*)>;

    explicit ServiceOrderListWidget(QWidget* parent = nullptr) : QListWidget(parent) {
        viewport()->installEventFilter(this);
    }

    void set_asset_drop_handler(AssetDropHandler handler) {
        asset_drop_handler_ = std::move(handler);
    }

    void set_content_drop_handler(ContentDropHandler handler) {
        content_drop_handler_ = std::move(handler);
    }

protected:
    bool handle_drop(const QMimeData* data, QPoint position) {
        if (asset_id_from_mime_data(data).has_value()) {
            return asset_drop_handler_ && asset_drop_handler_(data, position);
        }
        if (content_ref_from_mime_data(data).has_value()) {
            return content_drop_handler_ && content_drop_handler_(data);
        }
        return false;
    }

    bool accepts_drop(const QMimeData* data) const {
        return asset_id_from_mime_data(data).has_value() ||
               content_ref_from_mime_data(data).has_value();
    }

    bool eventFilter(QObject* watched, QEvent* event) override {
        if (watched == viewport() && event->type() == QEvent::Drop) {
            auto* drop_event = static_cast<QDropEvent*>(event);
            if (!handle_drop(drop_event->mimeData(), drop_event->position().toPoint())) {
                drop_event->ignore();
            } else {
                drop_event->acceptProposedAction();
            }
            return true;
        }
        if (watched == viewport() && event->type() == QEvent::DragEnter) {
            auto* drag_event = static_cast<QDragEnterEvent*>(event);
            if (accepts_drop(drag_event->mimeData())) {
                drag_event->acceptProposedAction();
            } else {
                drag_event->ignore();
            }
            return true;
        }
        if (watched == viewport() && event->type() == QEvent::DragMove) {
            auto* drag_event = static_cast<QDragMoveEvent*>(event);
            if (accepts_drop(drag_event->mimeData())) {
                drag_event->acceptProposedAction();
            } else {
                drag_event->ignore();
            }
            return true;
        }
        return QListWidget::eventFilter(watched, event);
    }

    bool viewportEvent(QEvent* event) override {
        if (event->type() == QEvent::DragEnter) {
            auto* drag_event = static_cast<QDragEnterEvent*>(event);
            if (accepts_drop(drag_event->mimeData())) {
                drag_event->acceptProposedAction();
            } else {
                drag_event->ignore();
            }
            return true;
        }
        if (event->type() == QEvent::DragMove) {
            auto* drag_event = static_cast<QDragMoveEvent*>(event);
            if (accepts_drop(drag_event->mimeData())) {
                drag_event->acceptProposedAction();
            } else {
                drag_event->ignore();
            }
            return true;
        }
        if (event->type() == QEvent::Drop) {
            auto* drop_event = static_cast<QDropEvent*>(event);
            if (!handle_drop(drop_event->mimeData(), drop_event->position().toPoint())) {
                drop_event->ignore();
            } else {
                drop_event->acceptProposedAction();
            }
            return true;
        }
        return QListWidget::viewportEvent(event);
    }

    void dragEnterEvent(QDragEnterEvent* event) override {
        if (accepts_drop(event->mimeData())) {
            event->acceptProposedAction();
        } else {
            event->ignore();
        }
    }

    void dragMoveEvent(QDragMoveEvent* event) override {
        if (accepts_drop(event->mimeData())) {
            event->acceptProposedAction();
        } else {
            event->ignore();
        }
    }

    void dropEvent(QDropEvent* event) override {
        if (!handle_drop(event->mimeData(), event->position().toPoint())) {
            event->ignore();
            return;
        }
        event->acceptProposedAction();
    }

private:
    AssetDropHandler asset_drop_handler_;
    ContentDropHandler content_drop_handler_;
};

}  // namespace

ServiceOrderWidget::ServiceOrderWidget(QWidget* parent) : QWidget(parent) {
    setObjectName(QStringLiteral("service-order"));
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    list_ = new ServiceOrderListWidget{this};
    list_->setObjectName(QStringLiteral("service-order-list"));
    list_->setAcceptDrops(true);
    list_->viewport()->setAcceptDrops(true);
    static_cast<ServiceOrderListWidget*>(list_)->set_asset_drop_handler(
        [this](const QMimeData* data, QPoint position) {
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
    static_cast<ServiceOrderListWidget*>(list_)->set_content_drop_handler(
        [this](const QMimeData* data) {
            const auto content = content_ref_from_mime_data(data);
            if (!content.has_value() || !content_drop_handler_) {
                return false;
            }
            content_drop_handler_(*content);
            return true;
        });
    QObject::connect(list_, &QListWidget::currentRowChanged, this, [this](int row) {
        if (row >= 0 && row < static_cast<int>(items_.size()) && selection_handler_) {
            selection_handler_(items_[static_cast<std::size_t>(row)].item.id);
        }
    });
    layout->addWidget(list_);

    auto* controls = new QHBoxLayout;
    auto* move_up = new QPushButton{QStringLiteral("Move Up"), this};
    move_up->setObjectName(QStringLiteral("service-order-move-up"));
    auto* move_down = new QPushButton{QStringLiteral("Move Down"), this};
    move_down->setObjectName(QStringLiteral("service-order-move-down"));
    auto* remove = new QPushButton{QStringLiteral("Remove"), this};
    remove->setObjectName(QStringLiteral("service-order-remove"));
    QObject::connect(move_up, &QPushButton::clicked, this, [this] {
        const auto row = list_->currentRow();
        if (row > 0 && row < static_cast<int>(items_.size()) && move_handler_) {
            move_handler_(items_[static_cast<std::size_t>(row)].item.id, row - 1);
        }
    });
    QObject::connect(move_down, &QPushButton::clicked, this, [this] {
        const auto row = list_->currentRow();
        if (row >= 0 && row + 1 < static_cast<int>(items_.size()) && move_handler_) {
            move_handler_(items_[static_cast<std::size_t>(row)].item.id, row + 1);
        }
    });
    QObject::connect(remove, &QPushButton::clicked, this, [this] {
        const auto row = list_->currentRow();
        if (row >= 0 && row < static_cast<int>(items_.size()) && remove_handler_) {
            remove_handler_(items_[static_cast<std::size_t>(row)].item.id);
        }
    });
    controls->addWidget(move_up);
    controls->addWidget(move_down);
    controls->addWidget(remove);
    layout->addLayout(controls);
}

void ServiceOrderWidget::set_items(std::vector<ServiceOrderView> items) {
    items_ = std::move(items);
    list_->clear();
    for (const auto& view : items_) {
        list_->addItem(view.label);
    }
}

void ServiceOrderWidget::select_row(int row) { list_->setCurrentRow(row); }

int ServiceOrderWidget::count() const noexcept { return list_->count(); }

}  // namespace shekinah::ui
