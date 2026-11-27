#pragma once

#include "shekinah/domain/model.hpp"
#include "shekinah/domain/result.hpp"

#include <sqlite3.h>

namespace shekinah::storage::song_repository_internal {

domain::Status write_reflow(sqlite3* database, const domain::Song& song);
domain::Status write_slides(sqlite3* database, const domain::Song& song);

}  // namespace shekinah::storage::song_repository_internal
