#pragma once

#include "shekinah/storage/song_repository.hpp"
#include "shekinah/storage/project_store.hpp"

namespace shekinah::storage {

class SqliteSongRepository final : public SongRepository {
public:
    explicit SqliteSongRepository(ProjectStore& store) : store_(store) {}

    domain::Result<domain::Song> save_song(const domain::Song& song) override;
    domain::Result<domain::Song> load_song(const domain::SongId& id) const override;
    domain::Result<application::SongReflowDocument> load_song_reflow(
        const domain::SongId& id) const override;
    domain::Result<std::vector<domain::Song>> list_songs() const override;
    domain::Result<std::size_t> song_count() const override;

private:
    ProjectStore& store_;
};

}  // namespace shekinah::storage
