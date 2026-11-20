#pragma once

#include "shekinah/domain/model.hpp"

#include <QWidget>

#include <functional>
#include <vector>

class QListWidget;

namespace shekinah::ui {

struct ServiceOrderView {
    domain::ServiceOrderItem item;
    QString label;
};

class ServiceOrderWidget final : public QWidget {
public:
    explicit ServiceOrderWidget(QWidget* parent = nullptr);

    void set_items(std::vector<ServiceOrderView> items);
    void set_selection_handler(std::function<void(domain::ServiceOrderItemId)> handler) {
        selection_handler_ = std::move(handler);
    }
    void select_row(int row);
    [[nodiscard]] int count() const noexcept;
    [[nodiscard]] const std::vector<ServiceOrderView>& items() const noexcept { return items_; }

private:
    QListWidget* list_{nullptr};
    std::vector<ServiceOrderView> items_;
    std::function<void(domain::ServiceOrderItemId)> selection_handler_;
};

}  // namespace shekinah::ui
