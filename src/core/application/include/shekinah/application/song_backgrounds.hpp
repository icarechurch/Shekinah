#pragma once

#include "shekinah/domain/model.hpp"
#include "shekinah/domain/result.hpp"

#include <optional>

namespace shekinah::application {

[[nodiscard]] domain::Result<std::optional<domain::BackgroundRef>> resolve_song_background(
    const domain::Song& song, const domain::CueId& cue);

domain::Status apply_background_from_slide(domain::Song& song,
                                           const domain::CueId& start,
                                           domain::BackgroundRef background);

void set_default_background(domain::Song& song, domain::BackgroundRef background);

void remove_asset_from_song(domain::Song& song, const domain::AssetId& asset);

}  // namespace shekinah::application
