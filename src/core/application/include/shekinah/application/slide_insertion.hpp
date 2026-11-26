#pragma once

#include "shekinah/domain/model.hpp"
#include "shekinah/domain/result.hpp"

#include <cstddef>

namespace shekinah::application {

domain::Result<domain::CueId> insert_asset_slide(domain::Song& song,
                                                 std::size_t index,
                                                 domain::BackgroundRef background);

domain::Result<domain::CueId> insert_asset_slide(domain::SermonDeck& deck,
                                                 std::size_t index,
                                                 domain::AssetId asset);

}  // namespace shekinah::application
