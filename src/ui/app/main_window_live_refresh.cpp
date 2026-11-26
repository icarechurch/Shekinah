#include "shekinah/ui/main_window.hpp"

#include "shekinah/application/live_slide_presenter.hpp"
#include "shekinah/media/audience_output.hpp"
#include "shekinah/storage/content_repository.hpp"
#include "shekinah/storage/project_store.hpp"
#include "shekinah/ui/cue_grid_widget.hpp"
#include "shekinah/ui/operator_page.hpp"
#include "shekinah/ui/program_status_widget.hpp"

#include <QString>

#include <filesystem>
#include <optional>

namespace shekinah::ui {

void ShekinahWindow::update_program_status() {
    const auto program = presentation_.program();
    std::optional<domain::CueId> selected_live_cue;
    const auto selected_item = presentation_.selected_item();
    if (program.live_cue.has_value() && selected_item.has_value() &&
        program.live_cue->content == selected_item->content) {
        selected_live_cue = program.live_cue->cue;
    }
    operator_page_->set_service_order_live_content(
        program.mode == domain::ProgramMode::cue && program.live_cue.has_value()
            ? std::optional<domain::ContentRef>{program.live_cue->content}
            : std::nullopt);
    operator_page_->cue_grid()->set_live_cue(std::move(selected_live_cue));
    operator_page_->program_status()->set_program(program);
    if (audience_output_) {
        audience_output_->present(presentation_.program());
    }
    present_live_media();
}

void ShekinahWindow::present_live_media() {
    if (!repository()) {
        operator_page_->program_status()->set_slide_preview({});
        operator_page_->program_status()->set_live_details({});
        return;
    }
    const auto& program = presentation_.program();
    if (program.mode != domain::ProgramMode::cue || !program.live_cue.has_value()) {
        operator_page_->program_status()->set_slide_preview({});
        operator_page_->program_status()->set_live_details({});
        if (audience_output_) {
            audience_output_->set_live_text({});
            audience_output_->clear_slide_image();
            audience_output_->clear_loop_video();
        }
        return;
    }

    const auto& cue = *program.live_cue;
    application::LiveSlidePresentation presented;
    if (cue.content.kind == domain::ContentKind::song) {
        const auto song = repository()->load_song(std::get<domain::SongId>(cue.content.id));
        if (song) {
            presented = application::build_live_song_presentation(*song, cue.cue);
        }
    } else {
        const auto deck = repository()->load_deck(std::get<domain::SermonId>(cue.content.id));
        if (deck) {
            presented = application::build_live_sermon_presentation(*deck, cue.cue);
        }
    }

    std::optional<domain::AssetId> asset_id = presented.background_asset;
    QString preview_text = QString::fromStdString(presented.slide_text);
    domain::FitMode preview_fit = presented.background_fit;
    LiveSlideDetails details{};
    details.item_title = QString::fromStdString(presented.item_title);
    details.section = QString::fromStdString(presented.section);
    details.slide_text = preview_text;
    details.next_section = QString::fromStdString(presented.next_section);
    details.next_summary = QString::fromStdString(presented.next_summary);
    details.slide_index = presented.slide_index;
    details.total_slides = presented.total_slides;
    if (asset_id.has_value()) {
        details.fit_mode = preview_fit;
    }

    if (audience_output_) {
        audience_output_->set_live_text(preview_text);
    }

    if (!asset_id.has_value()) {
        operator_page_->program_status()->set_slide_preview(std::move(preview_text));
        operator_page_->program_status()->set_live_details(std::move(details));
        if (audience_output_) {
            audience_output_->clear_slide_image();
            audience_output_->clear_loop_video();
        }
        return;
    }
    const auto asset = repository()->load_asset(*asset_id);
    if (!asset) {
        operator_page_->program_status()->set_slide_preview(std::move(preview_text));
        operator_page_->program_status()->set_live_details(std::move(details));
        return;
    }
    const auto filename = std::filesystem::path{asset->relative_path}.filename().string();
    details.has_background = true;
    details.is_video = (asset->kind == domain::AssetKind::video);
    details.asset_name = QString::fromStdString(filename.empty() ? asset->relative_path : filename);

    const auto path = store()->root() / asset->relative_path;
    if (asset->kind == domain::AssetKind::image) {
        operator_page_->program_status()->set_slide_preview(
            std::move(preview_text), QString::fromStdString(path.string()), preview_fit);
        operator_page_->program_status()->set_live_details(std::move(details));
        if (audience_output_) {
            audience_output_->clear_loop_video();
            audience_output_->load_slide_image(path, preview_fit);
        }
    } else {
        operator_page_->program_status()->set_slide_preview(std::move(preview_text));
        operator_page_->program_status()->set_live_details(std::move(details));
        if (!audience_output_) {
            return;
        }
        if (!live_media_admitted()) {
            return;
        }
        audience_output_->clear_slide_image();
        audience_output_->load_loop_video(path);
    }
}

}  // namespace shekinah::ui
