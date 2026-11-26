#include "shekinah/ui/main_window.hpp"

#include "shekinah/application/cue_presenter.hpp"
#include "shekinah/application/live_slide_presenter.hpp"
#include "shekinah/application/service_order_presenter.hpp"
#include "shekinah/media/audience_output.hpp"
#include "shekinah/storage/content_repository.hpp"
#include "shekinah/storage/project_store.hpp"
#include "shekinah/ui/cue_grid_widget.hpp"
#include "shekinah/ui/library_dock_widget.hpp"
#include "shekinah/ui/operator_page.hpp"
#include "shekinah/ui/program_status_widget.hpp"
#include "shekinah/ui/service_order_widget.hpp"

#include <QString>

#include <filesystem>
#include <optional>
#include <utility>
#include <vector>

namespace shekinah::ui {

void ShekinahWindow::refresh_library() {
    library_dock_->clear_contents();
    if (!repository()) {
        return;
    }

    library_dock_->set_asset_root(session_.root());
    const auto assets = repository()->list_assets();
    if (assets) {
        for (const auto& asset : *assets) {
            library_dock_->add_asset(asset);
        }
    }
    const auto songs = repository()->list_songs();
    if (songs) {
        for (const auto& song : *songs) {
            library_dock_->add_song(song);
        }
    }
    const auto sermons = repository()->list_decks();
    if (sermons) {
        for (const auto& sermon : *sermons) {
            library_dock_->add_sermon(sermon);
        }
    }
}

void ShekinahWindow::refresh_service_order() {
    presentation_.reset();
    if (!repository()) {
        operator_page_->set_service_order({});
        operator_page_->cue_grid()->set_cues({});
        operator_page_->set_active_presentation_title({});
        update_program_status();
        return;
    }

    const auto loaded = repository()->load_order(domain::ServiceOrderId{"default-service"});
    if (!loaded) {
        operator_page_->set_service_order({});
        operator_page_->cue_grid()->set_cues({});
        operator_page_->set_active_presentation_title({});
        update_program_status();
        return;
    }

    std::vector<ServiceOrderView> views;
    views.reserve(loaded->size());
    const auto program = presentation_.program();
    for (const auto& item : *loaded) {
        application::ServiceOrderItemPresentation presented;
        if (item.content.kind == domain::ContentKind::song) {
            const auto content_id = std::get<domain::SongId>(item.content.id);
            const auto song = repository()->load_song(content_id);
            if (song) {
                presentation_.register_song(*song);
                presented = application::present_service_order_item(
                    item, &*song, nullptr, program);
            } else {
                presented = application::present_service_order_item(
                    item, nullptr, nullptr, program);
            }
        } else {
            const auto content_id = std::get<domain::SermonId>(item.content.id);
            const auto sermon = repository()->load_deck(content_id);
            if (sermon) {
                presentation_.register_sermon(*sermon);
                presented = application::present_service_order_item(
                    item, nullptr, &*sermon, program);
            } else {
                presented = application::present_service_order_item(
                    item, nullptr, nullptr, program);
            }
        }
        views.push_back(ServiceOrderView{
            item,
            QString::fromStdString(presented.label),
            presented.slide_count,
            presented.has_background,
            presented.is_live,
        });
    }
    presentation_.set_service_order(*loaded);
    operator_page_->set_service_order(std::move(views));
    if (!loaded->empty()) {
        operator_page_->service_order_widget()->select_row(0);
    } else {
        operator_page_->cue_grid()->set_cues({});
        operator_page_->set_active_presentation_title({});
    }
    update_program_status();
}

void ShekinahWindow::refresh_cues() {
    const auto selected = presentation_.selected_item();
    if (!selected.has_value() || !repository()) {
        operator_page_->cue_grid()->set_cues({});
        operator_page_->program_status()->set_item_cues({}, {});
        operator_page_->set_active_presentation_title({});
        return;
    }

    const auto resolve_asset_path = [this](const domain::AssetId& asset_id)
        -> std::optional<std::filesystem::path> {
        if (!repository() || !store()) {
            return std::nullopt;
        }
        const auto asset = repository()->load_asset(asset_id);
        if (!asset) {
            return std::nullopt;
        }
        return store()->root() / asset->relative_path;
    };

    application::CuePresentationSet presented;
    if (selected->content.kind == domain::ContentKind::song) {
        const auto song = repository()->load_song(std::get<domain::SongId>(selected->content.id));
        if (!song) {
            operator_page_->cue_grid()->set_cues({});
            operator_page_->program_status()->set_item_cues({}, {});
            return;
        }
        presented = application::build_song_cue_presentation(*song, resolve_asset_path);
    } else {
        const auto sermon = repository()->load_deck(std::get<domain::SermonId>(selected->content.id));
        if (!sermon) {
            operator_page_->cue_grid()->set_cues({});
            operator_page_->program_status()->set_item_cues({}, {});
            return;
        }
        presented = application::build_sermon_cue_presentation(*sermon, resolve_asset_path);
    }

    std::vector<CueView> cues;
    cues.reserve(presented.cues.size());
    for (const auto& cue : presented.cues) {
        cues.push_back(CueView{
            cue.id,
            QString::fromStdString(cue.section),
            QString::fromStdString(cue.summary),
            cue.number,
            QString::fromStdString(cue.background_path.string()),
            cue.background_fit,
            QString::fromStdString(cue.footer_label),
            cue.show_text,
        });
    }

    std::vector<QuickCueItem> quick_cues;
    quick_cues.reserve(presented.quick_cues.size());
    for (const auto& cue : presented.quick_cues) {
        quick_cues.push_back(QuickCueItem{
            cue.id,
            cue.number,
            QString::fromStdString(cue.label),
            QString::fromStdString(cue.summary),
        });
    }

    auto item_title = QString::fromStdString(presented.title);
    operator_page_->set_active_presentation_title(item_title);
    operator_page_->cue_grid()->set_cues(std::move(cues));
    operator_page_->program_status()->set_item_cues(std::move(item_title), std::move(quick_cues));
}

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

