#include "shekinah/ui/main_window.hpp"

#include "shekinah/media/audience_output.hpp"
#include "shekinah/storage/content_repository.hpp"
#include "shekinah/ui/library_dock_widget.hpp"
#include "shekinah/ui/operator_page.hpp"
#include "shekinah/ui/program_status_widget.hpp"

#include <QScreen>

#include <utility>

namespace shekinah::ui {

void ShekinahWindow::configure_feature_handlers() {
    library_dock_->set_import_asset_handler([this] { import_asset(); });
    library_dock_->set_import_song_handler([this] { import_song_text(); });
    library_dock_->set_new_song_handler([this] { create_song(); });
    library_dock_->set_import_sermon_handler([this] { import_sermon_deck(); });
    library_dock_->set_new_sermon_handler([this] { create_sermon(); });
    library_dock_->set_edit_sermon_handler([this](domain::SermonId id) { edit_sermon(id); });
    library_dock_->set_edit_song_handler([this](domain::SongId id) { edit_song(id); });
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
    const auto add_content_to_presentation = [this](domain::ContentRef content) {
        if (!repository()) {
            return;
        }
        const auto appended = repository()->append_to_order(
            domain::ServiceOrderId{"default-service"}, std::move(content));
        if (appended) {
            refresh_service_order();
        }
    };
    library_dock_->set_add_content_handler(add_content_to_presentation);
}

void ShekinahWindow::configure_operator() {
    operator_page_->set_service_order_selection_handler([this](domain::ServiceOrderItemId item_id) {
        if (presentation_.select_service_item(item_id)) {
            refresh_cues();
        }
    });
    operator_page_->set_service_order_move_handler(
        [this](domain::ServiceOrderItemId item_id, std::int64_t target_ordinal) {
            if (repository() && repository()->move_in_order(item_id, target_ordinal)) {
                refresh_service_order();
            }
        });
    operator_page_->set_take_live_handler([this](domain::CueId cue) {
        const auto selected = presentation_.selected_item();
        if (selected.has_value() && presentation_.take_live(selected->id, cue)) {
            update_program_status();
        }
    });
    operator_page_->set_background_handler([this](domain::CueId cue) { choose_background(cue); });
    operator_page_->set_previous_handler([this] {
        if (presentation_.previous()) {
            update_program_status();
        }
    });
    operator_page_->set_next_handler([this] {
        if (presentation_.next()) {
            update_program_status();
        }
    });
    operator_page_->set_clear_handler([this] {
        presentation_.clear();
        update_program_status();
    });
    operator_page_->set_black_handler([this] {
        presentation_.black();
        update_program_status();
    });
    operator_page_->set_show_screen_handler([this](QScreen* screen) {
        if (screen == nullptr) {
            return;
        }
        audience_output()->show_fullscreen(screen);
        operator_page_->program_status()->set_output_status(
            QStringLiteral("Output: %1").arg(screen->name()),
            QStringLiteral("Audience full-screen"),
            true);
    });
    operator_page_->set_show_movable_handler([this] {
        audience_output()->show_movable();
        operator_page_->program_status()->set_output_status(
            QStringLiteral("Output: Window"),
            QStringLiteral("Audience window"),
            true);
    });
    operator_page_->set_hide_output_handler([this] {
        if (audience_output_) {
            audience_output_->hide();
        }
        operator_page_->program_status()->set_output_status(
            QString{},
            QStringLiteral("Output hidden"),
            false);
    });
    operator_page_->set_inspect_live_handler([this] {
        const auto& program = presentation_.program();
        if (program.live_cue.has_value()) {
            show_inspector(program.live_cue->cue);
        }
    });
    operator_page_->set_choose_background_handler([this] {
        const auto& program = presentation_.program();
        if (program.live_cue.has_value()) {
            choose_background(program.live_cue->cue);
        }
    });
}

}  // namespace shekinah::ui
