#pragma once

#include "shekinah/domain/model.hpp"
#include "shekinah/domain/result.hpp"
#include "shekinah/storage/project_store.hpp"

#include <filesystem>
#include <vector>

namespace shekinah::storage {

class SermonRepository;

class SermonDeckImporter {
public:
    SermonDeckImporter(ProjectStore& store, SermonRepository& sermons)
        : store_(store), sermons_(sermons) {}

    domain::Result<domain::SermonDeck> import_files(
        domain::SermonId id,
        std::string title,
        std::vector<std::filesystem::path> paths,
        bool preserve_order = false);

private:
    ProjectStore& store_;
    SermonRepository& sermons_;
};

}  // namespace shekinah::storage
