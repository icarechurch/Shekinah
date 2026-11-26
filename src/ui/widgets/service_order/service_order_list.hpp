#pragma once

#include "service_order_delegate.hpp"

#include "shekinah/ui/support/asset_drag.hpp"
#include "shekinah/ui/support/content_drag.hpp"
#include "shekinah/ui/support/theme.hpp"

#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QEvent>
#include <QKeyEvent>
#include <QListWidget>
#include <QMimeData>
#include <QPainter>

#include <functional>
#include <utility>

namespace shekinah::ui {
namespace {

class ServiceOrderListWidget final : public QListWidget {
public:
    using AssetDropHandler = std::function<bool(const QMimeData*, QPoint)>;
    using ContentDropHandler = std::function<bool(const QMimeData*)>;

    explicit ServiceOrderListWidget(QWidget* parent = nullptr) : QListWidget(parent) {
        viewport()->installEventFilter(this);
        setMouseTracking(true);
        setItemDelegate(new ServiceOrderItemDelegate{this});
    }

    void set_asset_drop_handler(AssetDropHandler handler) {
        asset_drop_handler_ = std::move(handler);
    }

    void set_content_drop_handler(ContentDropHandler handler) {
        content_drop_handler_ = std::move(handler);
    }

    void set_key_action_handlers(std::function<void(int)> move_delta,
                                 std::function<void()> remove_selected,
                                 std::function<void()> edit_selected) {
        key_move_delta_ = std::move(move_delta);
        key_remove_ = std::move(remove_selected);
        key_edit_ = std::move(edit_selected);
    }

    void set_viewport_resized_handler(std::function<void()> handler) {
        viewport_resized_ = std::move(handler);
    }

protected:
    void set_drop_hover_state(const QMimeData* data, QPoint pos) {
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

    void clear_drop_hover_state() {
        if (hovered_drop_row_ >= 0 && hovered_drop_row_ < count()) {
            item(hovered_drop_row_)->setData(kDropTargetRole, false);
        }
        hovered_drop_row_ = -1;
        content_drop_active_ = false;
        viewport()->update();
    }

    bool handle_drop(const QMimeData* data, QPoint position) {
        clear_drop_hover_state();
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
        if (watched == viewport()) {
            if (event->type() == QEvent::Resize) {
                if (viewport_resized_) {
                    viewport_resized_();
                }
            } else if (event->type() == QEvent::Drop) {
                auto* drop_event = static_cast<QDropEvent*>(event);
                if (!handle_drop(drop_event->mimeData(), drop_event->position().toPoint())) {
                    drop_event->ignore();
                } else {
                    drop_event->acceptProposedAction();
                }
                return true;
            } else if (event->type() == QEvent::DragEnter) {
                auto* drag_event = static_cast<QDragEnterEvent*>(event);
                if (accepts_drop(drag_event->mimeData())) {
                    set_drop_hover_state(drag_event->mimeData(), drag_event->position().toPoint());
                    drag_event->acceptProposedAction();
                } else {
                    clear_drop_hover_state();
                    drag_event->ignore();
                }
                return true;
            } else if (event->type() == QEvent::DragMove) {
                auto* drag_event = static_cast<QDragMoveEvent*>(event);
                if (accepts_drop(drag_event->mimeData())) {
                    set_drop_hover_state(drag_event->mimeData(), drag_event->position().toPoint());
                    drag_event->acceptProposedAction();
                } else {
                    clear_drop_hover_state();
                    drag_event->ignore();
                }
                return true;
            } else if (event->type() == QEvent::DragLeave) {
                clear_drop_hover_state();
                return true;
            }
        }
        return QListWidget::eventFilter(watched, event);
    }

    bool viewportEvent(QEvent* event) override {
        if (event->type() == QEvent::DragEnter) {
            auto* drag_event = static_cast<QDragEnterEvent*>(event);
            if (accepts_drop(drag_event->mimeData())) {
                set_drop_hover_state(drag_event->mimeData(), drag_event->position().toPoint());
                drag_event->acceptProposedAction();
            } else {
                clear_drop_hover_state();
                drag_event->ignore();
            }
            return true;
        }
        if (event->type() == QEvent::DragMove) {
            auto* drag_event = static_cast<QDragMoveEvent*>(event);
            if (accepts_drop(drag_event->mimeData())) {
                set_drop_hover_state(drag_event->mimeData(), drag_event->position().toPoint());
                drag_event->acceptProposedAction();
            } else {
                clear_drop_hover_state();
                drag_event->ignore();
            }
            return true;
        }
        if (event->type() == QEvent::DragLeave) {
            clear_drop_hover_state();
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

    void paintEvent(QPaintEvent* event) override {
        QListWidget::paintEvent(event);
        if (content_drop_active_) {
            QPainter p{viewport()};
            p.setRenderHint(QPainter::Antialiasing, true);
            p.setPen(QPen{ThemeManager::instance().palette().accent, 2.0});
            p.drawRoundedRect(viewport()->rect().adjusted(2, 2, -2, -2), 4.0, 4.0);
        }
    }

    void dragEnterEvent(QDragEnterEvent* event) override {
        if (accepts_drop(event->mimeData())) {
            set_drop_hover_state(event->mimeData(), event->position().toPoint());
            event->acceptProposedAction();
        } else {
            clear_drop_hover_state();
            event->ignore();
        }
    }

    void dragMoveEvent(QDragMoveEvent* event) override {
        if (accepts_drop(event->mimeData())) {
            set_drop_hover_state(event->mimeData(), event->position().toPoint());
            event->acceptProposedAction();
        } else {
            clear_drop_hover_state();
            event->ignore();
        }
    }

    void dragLeaveEvent(QDragLeaveEvent* event) override {
        clear_drop_hover_state();
        event->accept();
    }

    void dropEvent(QDropEvent* event) override {
        if (!handle_drop(event->mimeData(), event->position().toPoint())) {
            event->ignore();
            return;
        }
        event->acceptProposedAction();
    }

    void keyPressEvent(QKeyEvent* event) override {
        if ((event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace) && key_remove_) {
            key_remove_();
            event->accept();
            return;
        }
        if (event->modifiers().testFlag(Qt::AltModifier) && event->key() == Qt::Key_Up && key_move_delta_) {
            key_move_delta_(-1);
            event->accept();
            return;
        }
        if (event->modifiers().testFlag(Qt::AltModifier) && event->key() == Qt::Key_Down && key_move_delta_) {
            key_move_delta_(1);
            event->accept();
            return;
        }
        if ((event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) &&
            event->modifiers().testFlag(Qt::ControlModifier) && key_edit_) {
            key_edit_();
            event->accept();
            return;
        }
        QListWidget::keyPressEvent(event);
    }

private:
    AssetDropHandler asset_drop_handler_;
    ContentDropHandler content_drop_handler_;
    std::function<void(int)> key_move_delta_;
    std::function<void()> key_remove_;
    std::function<void()> key_edit_;
    std::function<void()> viewport_resized_;
    int hovered_drop_row_{-1};
    bool content_drop_active_{false};
};


}  // namespace
}  // namespace shekinah::ui
