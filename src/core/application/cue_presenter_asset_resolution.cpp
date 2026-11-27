#include "cue_presenter_asset_resolution.hpp"

namespace shekinah::application::internal {

std::optional<std::filesystem::path> resolve_asset_path(
    const AssetPathResolver& resolver, const domain::AssetId& asset) {
    if (!resolver) {
        return std::nullopt;
    }
    return resolver(asset);
}

}  // namespace shekinah::application::internal
