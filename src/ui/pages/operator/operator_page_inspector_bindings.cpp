#include "shekinah/ui/operator_page.hpp"

#include "shekinah/ui/inspector_dialog.hpp"

#include <utility>

namespace shekinah::ui {

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

}  // namespace shekinah::ui
