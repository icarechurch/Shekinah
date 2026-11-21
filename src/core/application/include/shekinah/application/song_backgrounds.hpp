#pragma once

#include "shekinah/domain/model.hpp"
#include "shekinah/domain/result.hpp"

namespace shekinah::application {

domain::Status apply_background_from_slide(domain::Song& song,
                                           const domain::CueId& start,
                                           domain::BackgroundRef background);

void set_default_background(domain::Song& song, domain::BackgroundRef background);

}  // namespace shekinah::application
