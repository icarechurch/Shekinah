#pragma once

#include "shekinah/application/song_reflow.hpp"

namespace shekinah::application {

void normalize_song_cue_ids(domain::Song& song);

[[nodiscard]] SongReflowDocument reflow_document_for_song(const domain::Song& song);

}  // namespace shekinah::application
