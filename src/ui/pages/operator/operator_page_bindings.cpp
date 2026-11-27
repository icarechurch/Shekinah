#include "shekinah/ui/operator_page.hpp"

#include "shekinah/ui/cue_grid_widget.hpp"
#include "shekinah/ui/inspector_dialog.hpp"
#include "shekinah/ui/program_status_widget.hpp"
#include "shekinah/ui/service_order_widget.hpp"

#include <QLabel>

namespace shekinah::ui {

void OperatorPage::set_active_presentation_title(const QString& title) {
    const QString trimmed = title.trimmed();
    if (trimmed.isEmpty()) {
        presentation_item_label_->clear();
        presentation_item_label_->setVisible(false);
    } else {
        presentation_item_label_->setText(QStringLiteral("·  %1").arg(trimmed));
        presentation_item_label_->setVisible(true);
    }
}

void OperatorPage::set_cue_asset_drop_handler(
    std::function<void(domain::CueId, domain::AssetId)> handler) {
    cue_grid_->set_asset_drop_handler(std::move(handler));
}

void OperatorPage::set_cue_asset_insert_handler(
    std::function<void(std::size_t, domain::AssetId)> handler) {
    cue_grid_->set_asset_insert_handler(std::move(handler));
}

void OperatorPage::set_authoring_handlers(std::function<void()> new_song,
                                          std::function<void()> import_song,
                                          std::function<void()> import_sermon,
                                          std::function<void()> import_asset) {
    new_song_handler_ = new_song;
    import_song_handler_ = import_song;
    import_sermon_handler_ = import_sermon;
    service_order_->set_authoring_handlers(
        new_song_handler_, import_song_handler_, new_sermon_handler_, import_sermon_handler_);
    cue_grid_->set_authoring_handlers(
        std::move(new_song), std::move(import_song), std::move(import_sermon), std::move(import_asset));
}

void OperatorPage::set_inspector_handler(std::function<void(domain::CueId)> handler) {
    inspector_handler_ = std::move(handler);
}

void OperatorPage::set_cue_asset_name_handler(std::function<QString(domain::CueId)> handler) {
    cue_asset_name_handler_ = std::move(handler);
}

void OperatorPage::set_remove_asset_handler(std::function<void(domain::CueId)> handler) {
    remove_asset_handler_ = std::move(handler);
}

void OperatorPage::set_edit_slide_handler(std::function<void(domain::CueId)> handler) {
    edit_slide_handler_ = std::move(handler);
}

void OperatorPage::set_duplicate_slide_handler(std::function<void(domain::CueId)> handler) {
    duplicate_slide_handler_ = std::move(handler);
}

void OperatorPage::set_delete_slide_handler(std::function<void(domain::CueId)> handler) {
    delete_slide_handler_ = std::move(handler);
}

void OperatorPage::set_inspector_save_handler(InspectorDialog::SaveHandler handler) {
    inspector_->set_save_handler(std::move(handler));
}

void OperatorPage::set_take_live_handler(std::function<void(domain::CueId)> handler) {
    cue_grid_->set_take_live_handler(handler);
    program_status_->set_take_cue_handler(std::move(handler));
}

void OperatorPage::set_background_handler(std::function<void(domain::CueId)> handler) {
    background_handler_ = std::move(handler);
}

void OperatorPage::set_previous_handler(std::function<void()> handler) {
    program_status_->set_previous_handler(std::move(handler));
}

void OperatorPage::set_next_handler(std::function<void()> handler) {
    program_status_->set_next_handler(std::move(handler));
}

void OperatorPage::set_clear_handler(std::function<void()> handler) {
    program_status_->set_clear_handler(std::move(handler));
}

void OperatorPage::set_black_handler(std::function<void()> handler) {
    program_status_->set_black_handler(std::move(handler));
}

void OperatorPage::set_show_screen_handler(std::function<void(QScreen*)> handler) {
    program_status_->set_show_screen_handler(std::move(handler));
}

void OperatorPage::set_show_movable_handler(std::function<void()> handler) {
    program_status_->set_show_movable_handler(std::move(handler));
}

void OperatorPage::set_hide_output_handler(std::function<void()> handler) {
    program_status_->set_hide_output_handler(std::move(handler));
}

void OperatorPage::set_inspect_live_handler(std::function<void()> handler) {
    program_status_->set_inspect_live_handler(std::move(handler));
}

void OperatorPage::set_choose_background_handler(std::function<void()> handler) {
    program_status_->set_choose_background_handler(std::move(handler));
}

}  // namespace shekinah::ui
