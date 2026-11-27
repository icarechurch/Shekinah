#include "shekinah/ui/main_window.hpp"

#include "shekinah/storage/content_repository.hpp"
#include "shekinah/ui/operator_page.hpp"

#include <utility>

namespace shekinah::ui {

void ShekinahWindow::configure_operator_feature_handlers() {
    operator_page_->set_authoring_handlers(
        [this] { create_song(); },
        [this] { import_song_text(); },
        [this] { import_sermon_deck(); },
        [this] { import_asset(); });
    operator_page_->set_service_order_new_sermon_handler([this] { create_sermon(); });
    operator_page_->set_service_order_edit_handler([this](domain::ContentRef content) {
        if (content.kind == domain::ContentKind::song) {
            edit_song(std::get<domain::SongId>(content.id));
        } else {
            edit_sermon(std::get<domain::SermonId>(content.id));
        }
    });
    operator_page_->set_inspector_handler([this](domain::CueId cue) { show_inspector(cue); });
    operator_page_->set_edit_slide_handler([this](domain::CueId cue) { show_inspector(cue); });
    operator_page_->set_cue_asset_name_handler(
        [this](domain::CueId cue) { return asset_name_for_cue(cue); });
    operator_page_->set_remove_asset_handler(
        [this](domain::CueId cue) { remove_asset_from_song(cue); });
    operator_page_->set_duplicate_slide_handler([this](domain::CueId cue) { duplicate_slide(cue); });
    operator_page_->set_delete_slide_handler([this](domain::CueId cue) { delete_slide(cue); });
    operator_page_->set_service_order_asset_drop_handler(
        [this](domain::ContentRef content, domain::AssetId asset) {
            apply_asset_to_song(std::move(content), std::move(asset));
        });
    operator_page_->set_service_order_content_drop_handler([this](domain::ContentRef content) {
        if (!repository()) {
            return;
        }
        const auto appended = repository()->append_to_order(
            domain::ServiceOrderId{"default-service"}, std::move(content));
        if (appended) {
            refresh_service_order();
        }
    });
    operator_page_->set_service_order_remove_handler([this](domain::ServiceOrderItemId item) {
        if (repository() && repository()->remove_from_order(item)) {
            refresh_service_order();
        }
    });
    operator_page_->set_cue_asset_drop_handler(
        [this](domain::CueId cue, domain::AssetId asset) {
            apply_asset_to_slide(std::move(cue), std::move(asset));
        });
    operator_page_->set_cue_asset_insert_handler(
        [this](std::size_t index, domain::AssetId asset) {
            insert_asset_at_slide_position(index, std::move(asset));
        });
    operator_page_->set_inspector_save_handler(
        [this](domain::CueId cue, std::string section, std::vector<std::string> lines, domain::FitMode fit) {
            save_inspected_slide(std::move(cue), std::move(section), std::move(lines), fit);
        });
}

}  // namespace shekinah::ui
