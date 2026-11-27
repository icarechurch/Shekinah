#pragma once

#include "shekinah/domain/model.hpp"
#include "shekinah/domain/result.hpp"

#include <vector>

namespace shekinah::storage {

class SermonRepository {
public:
    virtual ~SermonRepository() = default;

    virtual domain::Result<domain::SermonDeck> save_deck(const domain::SermonDeck& deck) = 0;
    virtual domain::Result<domain::SermonDeck> load_deck(const domain::SermonId& id) const = 0;
    virtual domain::Result<std::vector<domain::SermonDeck>> list_decks() const = 0;
};

}  // namespace shekinah::storage
