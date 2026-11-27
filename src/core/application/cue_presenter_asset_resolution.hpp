#pragma once

#include "shekinah/application/cue_presenter.hpp"

namespace shekinah::application::internal {

[[nodiscard]] std::optional<std::filesystem::path> resolve_asset_path(
    const AssetPathResolver& resolver, const domain::AssetId& asset);

}  // namespace shekinah::application::internal
