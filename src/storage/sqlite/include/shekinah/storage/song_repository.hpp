#pragma once

#include "shekinah/application/song_reflow.hpp"
#include "shekinah/domain/model.hpp"
#include "shekinah/domain/result.hpp"

#include <cstddef>
#include <vector>

namespace shekinah::storage {

class SongRepository {
public:
    virtual ~SongRepository() = default;

    virtual domain::Result<domain::Song> save_song(const domain::Song& song) = 0;
    virtual domain::Result<domain::Song> load_song(const domain::SongId& id) const = 0;
    virtual domain::Result<application::SongReflowDocument> load_song_reflow(
        const domain::SongId& id) const = 0;
    virtual domain::Result<std::vector<domain::Song>> list_songs() const = 0;
    virtual domain::Result<std::size_t> song_count() const = 0;
};

}  // namespace shekinah::storage
