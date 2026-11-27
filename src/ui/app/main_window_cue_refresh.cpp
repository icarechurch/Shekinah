#include "shekinah/ui/main_window.hpp"

#include "shekinah/application/cue_presenter.hpp"
#include "main_window_cue_views.hpp"
#include "shekinah/storage/asset_repository.hpp"
#include "shekinah/storage/project_store.hpp"
#include "shekinah/storage/sermon_repository.hpp"
#include "shekinah/storage/song_repository.hpp"
#include "shekinah/ui/cue_grid_widget.hpp"
#include "shekinah/ui/operator_page.hpp"
#include "shekinah/ui/program_status_widget.hpp"

#include <filesystem>
#include <optional>

namespace shekinah::ui {

void ShekinahWindow::refresh_cues() {
    const auto selected = presentation_.selected_item();
    if (!selected.has_value() || !assets() || !songs() || !sermons()) {
        operator_page_->cue_grid()->set_cues({});
        operator_page_->program_status()->set_item_cues({}, {});
        operator_page_->set_active_presentation_title({});
        return;
    }

    const auto resolve_asset_path = [this](const domain::AssetId& asset_id)
        -> std::optional<std::filesystem::path> {
        if (!assets() || !store()) {
            return std::nullopt;
        }
        const auto asset = assets()->load_asset(asset_id);
        if (!asset) {
            return std::nullopt;
        }
        return store()->root() / asset->relative_path;
    };

    application::CuePresentationSet presented;
    if (selected->content.kind == domain::ContentKind::song) {
        const auto song = songs()->load_song(std::get<domain::SongId>(selected->content.id));
        if (!song) {
            operator_page_->cue_grid()->set_cues({});
            operator_page_->program_status()->set_item_cues({}, {});
            return;
        }
        presented = application::build_song_cue_presentation(*song, resolve_asset_path);
    } else {
        const auto sermon = sermons()->load_deck(std::get<domain::SermonId>(selected->content.id));
        if (!sermon) {
            operator_page_->cue_grid()->set_cues({});
            operator_page_->program_status()->set_item_cues({}, {});
            return;
        }
        presented = application::build_sermon_cue_presentation(*sermon, resolve_asset_path);
    }

    auto views = main_window_internal::make_cue_views(presented);
    operator_page_->set_active_presentation_title(views.title);
    operator_page_->cue_grid()->set_cues(std::move(views.cues));
    operator_page_->program_status()->set_item_cues(
        std::move(views.title), std::move(views.quick_cues));
}

}  // namespace shekinah::ui
