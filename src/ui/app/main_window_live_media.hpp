#pragma once

#include "shekinah/application/live_slide_presenter.hpp"
#include "shekinah/domain/model.hpp"
#include "shekinah/storage/asset_repository.hpp"
#include "shekinah/storage/sermon_repository.hpp"
#include "shekinah/storage/song_repository.hpp"
#include "shekinah/ui/program_status_types.hpp"

#include <filesystem>
#include <optional>

namespace shekinah::ui::main_window_internal {

struct LiveMediaSnapshot {
    application::LiveSlidePresentation presentation;
    std::optional<domain::AssetRecord> asset;
    std::filesystem::path asset_path;
};

[[nodiscard]] std::optional<LiveMediaSnapshot> resolve_live_media(
    const storage::SongRepository& songs,
    const storage::SermonRepository& sermons,
    const storage::AssetRepository& assets,
    const domain::ProgramState& program,
    const std::filesystem::path& project_root);

[[nodiscard]] LiveSlideDetails make_live_details(
    const application::LiveSlidePresentation& presentation,
    const std::optional<domain::AssetRecord>& asset);

}  // namespace shekinah::ui::main_window_internal
