#pragma once

#include "shekinah/domain/model.hpp"
#include "shekinah/domain/result.hpp"

#include <filesystem>
#include <string>
#include <utility>

namespace shekinah::storage::asset_importer_internal {

domain::Result<std::pair<domain::AssetKind, std::string>> classify(
    const std::filesystem::path& source);
domain::Result<std::string> sha256_file(const std::filesystem::path& source);
const char* kind_name(domain::AssetKind kind);

}  // namespace shekinah::storage::asset_importer_internal
