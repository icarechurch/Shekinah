#pragma once

#include "shekinah/storage/project_store.hpp"
#include "shekinah/storage/sermon_repository.hpp"

namespace shekinah::storage {

class SqliteSermonRepository final : public SermonRepository {
public:
    explicit SqliteSermonRepository(ProjectStore& store) : store_(store) {}

    domain::Result<domain::SermonDeck> save_deck(const domain::SermonDeck& deck) override;
    domain::Result<domain::SermonDeck> load_deck(const domain::SermonId& id) const override;
    domain::Result<std::vector<domain::SermonDeck>> list_decks() const override;

private:
    ProjectStore& store_;
};

}  // namespace shekinah::storage
