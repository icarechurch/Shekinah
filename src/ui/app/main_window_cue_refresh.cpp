#include "shekinah/ui/main_window.hpp"

#include "shekinah/application/cue_presenter.hpp"
#include "shekinah/storage/content_repository.hpp"
#include "shekinah/storage/project_store.hpp"
#include "shekinah/ui/cue_grid_widget.hpp"
#include "shekinah/ui/operator_page.hpp"
#include "shekinah/ui/program_status_widget.hpp"

#include <QString>

#include <filesystem>
#include <optional>
#include <vector>

namespace shekinah::ui {

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

}  // namespace shekinah::ui
