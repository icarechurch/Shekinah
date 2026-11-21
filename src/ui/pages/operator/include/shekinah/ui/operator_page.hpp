#pragma once

#include "shekinah/domain/model.hpp"
#include "shekinah/ui/inspector_dialog.hpp"

#include <QString>
#include <QWidget>

#include <cstdint>
#include <functional>
#include <vector>

class QListWidget;
class QMenu;

namespace shekinah::ui {

class CueGridWidget;
class InspectorDialog;
class ProgramStatusWidget;
class ServiceOrderWidget;
struct ServiceOrderView;

class OperatorPage final : public QWidget {
public:
    explicit OperatorPage(QWidget* parent = nullptr);

    void set_service_order(std::vector<ServiceOrderView> items);
    void set_service_order_selection_handler(std::function<void(domain::ServiceOrderItemId)> handler);
    void set_service_order_move_handler(
        std::function<void(domain::ServiceOrderItemId, std::int64_t)> handler);
    void set_service_order_asset_drop_handler(
        std::function<void(domain::ContentRef, domain::AssetId)> handler);
    void set_cue_asset_drop_handler(std::function<void(domain::CueId, domain::AssetId)> handler);
    void set_authoring_handlers(std::function<void()> new_song,
                                std::function<void()> import_song,
                                std::function<void()> import_sermon,
                                std::function<void()> import_asset);
    void set_inspector_handler(std::function<void(domain::CueId)> handler);
    void set_cue_asset_name_handler(std::function<QString(domain::CueId)> handler);
    void set_remove_asset_handler(std::function<void(domain::CueId)> handler);
    void set_edit_slide_handler(std::function<void(domain::CueId)> handler);
    void set_duplicate_slide_handler(std::function<void(domain::CueId)> handler);
    void set_delete_slide_handler(std::function<void(domain::CueId)> handler);
    void set_inspector_save_handler(InspectorDialog::SaveHandler handler);
    void set_take_live_handler(std::function<void(domain::CueId)> handler);
    void set_background_handler(std::function<void(domain::CueId)> handler);
    void set_previous_handler(std::function<void()> handler);
    void set_next_handler(std::function<void()> handler);
    void set_clear_handler(std::function<void()> handler);
    void set_black_handler(std::function<void()> handler);
    [[nodiscard]] ServiceOrderWidget* service_order_widget() const noexcept { return service_order_; }
    [[nodiscard]] CueGridWidget* cue_grid() const noexcept { return cue_grid_; }
    [[nodiscard]] ProgramStatusWidget* program_status() const noexcept { return program_status_; }
    [[nodiscard]] InspectorDialog* inspector() const noexcept { return inspector_; }
    QMenu* create_context_menu(const domain::CueId& cue);

private:
    ServiceOrderWidget* service_order_{nullptr};
    CueGridWidget* cue_grid_{nullptr};
    ProgramStatusWidget* program_status_{nullptr};
    InspectorDialog* inspector_{nullptr};
    std::function<void(domain::CueId)> background_handler_;
    std::function<void(domain::CueId)> inspector_handler_;
    std::function<QString(domain::CueId)> cue_asset_name_handler_;
    std::function<void(domain::CueId)> remove_asset_handler_;
    std::function<void(domain::CueId)> edit_slide_handler_;
    std::function<void(domain::CueId)> duplicate_slide_handler_;
    std::function<void(domain::CueId)> delete_slide_handler_;
};

}  // namespace shekinah::ui
