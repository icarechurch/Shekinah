#pragma once

#include <filesystem>

#include "shekinah/domain/model.hpp"
#include "shekinah/domain/result.hpp"

namespace shekinah::storage {

class ProjectStore;

class AssetImporter {
public:
    explicit AssetImporter(ProjectStore& store) : store_(store) {}

    domain::Result<domain::AssetRecord> import_file(const std::filesystem::path& source);

private:
    ProjectStore& store_;
};

}  // namespace shekinah::storage
