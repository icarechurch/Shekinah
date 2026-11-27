#pragma once

#include "shekinah/storage/content_archive_repository.hpp"
#include "shekinah/storage/project_store.hpp"

namespace shekinah::storage {

class SqliteContentArchiveRepository final : public ContentArchiveRepository {
public:
    explicit SqliteContentArchiveRepository(ProjectStore& store) : store_(store) {}

    domain::Status archive_song(const domain::SongId& id) override;
    domain::Status archive_deck(const domain::SermonId& id) override;

private:
    ProjectStore& store_;
};

}  // namespace shekinah::storage
