#pragma once

#include "shekinah/ui/service_order_view.hpp"

#include <QString>
#include <QWidget>

#include <cstdint>
#include <functional>
#include <optional>
#include <vector>

class QLabel;
class QLineEdit;
class QListWidget;
class QPushButton;
class QVBoxLayout;

namespace shekinah::ui {

class ServiceOrderWidget final : public QWidget {
public:
    using AssetDropHandler = std::function<void(domain::ContentRef, domain::AssetId)>;
    using ContentDropHandler = std::function<void(domain::ContentRef)>;

    explicit ServiceOrderWidget(QWidget* parent = nullptr);

    void set_items(std::vector<ServiceOrderView> items);
    void update_item_slide_count(const domain::ContentRef& content, int slide_count);
    void set_live_content(std::optional<domain::ContentRef> live_content);
    void set_selection_handler(std::function<void(domain::ServiceOrderItemId)> handler) {
        selection_handler_ = std::move(handler);
    }
    void set_move_handler(std::function<void(domain::ServiceOrderItemId, std::int64_t)> handler) {
        move_handler_ = std::move(handler);
    }
    void set_asset_drop_handler(AssetDropHandler handler) {
        asset_drop_handler_ = std::move(handler);
    }
    void set_content_drop_handler(ContentDropHandler handler) {
        content_drop_handler_ = std::move(handler);
    }
    void set_remove_handler(std::function<void(domain::ServiceOrderItemId)> handler) {
        remove_handler_ = std::move(handler);
    }
    void set_edit_handler(std::function<void(domain::ContentRef)> handler) {
        edit_handler_ = std::move(handler);
    }
    void set_authoring_handlers(std::function<void()> new_song,
                                std::function<void()> import_song,
                                std::function<void()> new_sermon,
                                std::function<void()> import_sermon);
    void select_row(int row);
    [[nodiscard]] int count() const noexcept;
    [[nodiscard]] const std::vector<ServiceOrderView>& items() const noexcept { return items_; }

private:
    void setup_header(QVBoxLayout* layout);
    void setup_list(QVBoxLayout* layout);
    void setup_controls(QVBoxLayout* layout);
    void setup_filter(QVBoxLayout* layout);
    void apply_theme();
    void apply_theme_icons();
    void apply_filter();
    void update_controls_state();
    void update_empty_state();
    void move_selected(int delta);
    void remove_selected();
    void edit_selected();
    void open_add_menu();
    void open_context_menu(const QPoint& viewport_pos);

    QListWidget* list_{nullptr};
    QLabel* count_badge_{nullptr};
    QPushButton* add_button_{nullptr};
    QPushButton* move_up_button_{nullptr};
    QPushButton* move_down_button_{nullptr};
    QPushButton* edit_button_{nullptr};
    QPushButton* remove_button_{nullptr};
    QLineEdit* filter_input_{nullptr};
    QWidget* empty_overlay_{nullptr};
    QLabel* empty_icon_label_{nullptr};
    QLabel* empty_title_label_{nullptr};
    QLabel* empty_hint_label_{nullptr};
    std::vector<ServiceOrderView> items_;
    std::optional<domain::ContentRef> live_content_;
    std::function<void(domain::ServiceOrderItemId)> selection_handler_;
    std::function<void(domain::ServiceOrderItemId, std::int64_t)> move_handler_;
    AssetDropHandler asset_drop_handler_;
    ContentDropHandler content_drop_handler_;
    std::function<void(domain::ServiceOrderItemId)> remove_handler_;
    std::function<void(domain::ContentRef)> edit_handler_;
    std::function<void()> new_song_handler_;
    std::function<void()> import_song_handler_;
    std::function<void()> new_sermon_handler_;
    std::function<void()> import_sermon_handler_;
};

}  // namespace shekinah::ui
