#pragma once

#include "shekinah/domain/model.hpp"
#include "shekinah/domain/result.hpp"
#include "shekinah/storage/project_store.hpp"

#include <filesystem>
#include <vector>

namespace shekinah::storage {

class SermonDeckImporter {
public:
    explicit SermonDeckImporter(ProjectStore& store) : store_(store) {}

    domain::Result<domain::SermonDeck> import_pngs(
        domain::SermonId id,
        std::string title,
        std::vector<std::filesystem::path> paths,
        bool preserve_order = false);

private:
    ProjectStore& store_;
};

}  // namespace shekinah::storage
