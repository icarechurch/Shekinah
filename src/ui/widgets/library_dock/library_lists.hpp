#pragma once

#include "library_components.hpp"

namespace shekinah::ui {
namespace library_internal {

class AssetListWidget final : public QListWidget {
public:
    using QListWidget::QListWidget;

protected:
    QMimeData* mimeData(const QList<QListWidgetItem*>& items) const override {
        if (items.size() != 1 || items.front() == nullptr) {
            return nullptr;
        }
        const auto id = items.front()->data(kContentIdRole).toString();
        return id.isEmpty() ? nullptr : make_asset_mime_data(domain::AssetId{id.toStdString()});
    }
};

class ContentListWidget final : public QListWidget {
public:
    ContentListWidget(domain::ContentKind kind, QWidget* parent) : QListWidget(parent), kind_(kind) {}

    void set_action_handlers(std::function<void(QListWidgetItem*)> add_handler,
                             std::function<void(QListWidgetItem*)> edit_handler) {
        add_handler_ = std::move(add_handler);
        edit_handler_ = std::move(edit_handler);
    }

protected:
    QMimeData* mimeData(const QList<QListWidgetItem*>& items) const override {
        if (items.size() != 1 || items.front() == nullptr) {
            return nullptr;
        }
        const auto id = items.front()->data(kContentIdRole).toString();
        if (id.isEmpty()) {
            return nullptr;
        }
        return make_content_mime_data(kind_ == domain::ContentKind::song
                                          ? domain::ContentRef::song(id.toStdString())
                                          : domain::ContentRef::sermon(id.toStdString()));
    }

    void mouseReleaseEvent(QMouseEvent* event) override {
        if (event->button() == Qt::LeftButton && add_handler_) {
            const QPoint pos = event->position().toPoint();
            if (auto* clicked = itemAt(pos); clicked != nullptr) {
                const QRect card = visualItemRect(clicked).adjusted(3, 3, -3, -3);
                if (quick_add_rect_for_card(card).contains(pos)) {
                    QListWidget::mouseReleaseEvent(event);
                    add_handler_(clicked);
                    return;
                }
            }
        }
        QListWidget::mouseReleaseEvent(event);
    }

    void keyPressEvent(QKeyEvent* event) override {
        if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
            if (auto* item = currentItem(); item != nullptr) {
                if (event->modifiers().testFlag(Qt::ControlModifier)) {
                    if (edit_handler_) {
                        edit_handler_(item);
                        event->accept();
                        return;
                    }
                } else if (add_handler_) {
                    add_handler_(item);
                    event->accept();
                    return;
                }
            }
        }
        QListWidget::keyPressEvent(event);
    }

private:
    domain::ContentKind kind_;
    std::function<void(QListWidgetItem*)> add_handler_;
    std::function<void(QListWidgetItem*)> edit_handler_;
};

}  // namespace library_internal
}  // namespace shekinah::ui
