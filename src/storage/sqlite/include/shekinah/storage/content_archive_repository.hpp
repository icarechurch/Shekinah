#pragma once

#include "shekinah/domain/model.hpp"
#include "shekinah/domain/result.hpp"

namespace shekinah::storage {

class ContentArchiveRepository {
public:
    virtual ~ContentArchiveRepository() = default;

    virtual domain::Status archive_song(const domain::SongId& id) = 0;
    virtual domain::Status archive_deck(const domain::SermonId& id) = 0;
};

}  // namespace shekinah::storage
