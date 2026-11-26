#pragma once

#include <QListWidget>
#include <QPoint>

#include <functional>

class QDragEnterEvent;
class QDragLeaveEvent;
class QDragMoveEvent;
class QDropEvent;
class QEvent;
class QKeyEvent;
class QMimeData;
class QObject;
class QPaintEvent;

namespace shekinah::ui::service_order_internal {

class ServiceOrderListWidget final : public QListWidget {
public:
    using AssetDropHandler = std::function<bool(const QMimeData*, QPoint)>;
    using ContentDropHandler = std::function<bool(const QMimeData*)>;

    explicit ServiceOrderListWidget(QWidget* parent = nullptr);

    void set_asset_drop_handler(AssetDropHandler handler);
    void set_content_drop_handler(ContentDropHandler handler);
    void set_key_action_handlers(std::function<void(int)> move_delta,
                                 std::function<void()> remove_selected,
                                 std::function<void()> edit_selected);
    void set_viewport_resized_handler(std::function<void()> handler);

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;
    bool viewportEvent(QEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragMoveEvent(QDragMoveEvent* event) override;
    void dragLeaveEvent(QDragLeaveEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private:
    void set_drop_hover_state(const QMimeData* data, QPoint pos);
    void clear_drop_hover_state();
    bool handle_drop(const QMimeData* data, QPoint position);
    [[nodiscard]] bool accepts_drop(const QMimeData* data) const;

    AssetDropHandler asset_drop_handler_;
    ContentDropHandler content_drop_handler_;
    std::function<void(int)> key_move_delta_;
    std::function<void()> key_remove_;
    std::function<void()> key_edit_;
    std::function<void()> viewport_resized_;
    int hovered_drop_row_{-1};
    bool content_drop_active_{false};
};

}  // namespace shekinah::ui::service_order_internal
