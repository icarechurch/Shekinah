#pragma once

#include "shekinah/domain/result.hpp"

#include <filesystem>
#include <vector>

namespace shekinah::storage::sermon_deck_importer_internal {

domain::Result<std::vector<std::filesystem::path>> prepare_import_paths(
    std::vector<std::filesystem::path> paths, bool preserve_order);

}  // namespace shekinah::storage::sermon_deck_importer_internal
