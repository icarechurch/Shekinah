#include "shekinah/storage/sermon_deck_importer.hpp"

#include "internal/sermon_deck_importer_order.hpp"

#include "shekinah/storage/asset_importer.hpp"
#include "shekinah/storage/project_store.hpp"
#include "shekinah/storage/sermon_repository.hpp"

#include <string>
#include <utility>

namespace shekinah::storage {

domain::Result<domain::SermonDeck> SermonDeckImporter::import_files(
    domain::SermonId id,
    std::string title,
    std::vector<std::filesystem::path> paths,
    bool preserve_order) {
    if (id.empty() || title.empty() || paths.empty()) {
        return domain::Result<domain::SermonDeck>::fail(
            {domain::ErrorCode::invalid_input,
             "A Sermon deck needs an id, title, and at least one media file"});
    }

    const auto prepared = sermon_deck_importer_internal::prepare_import_paths(
        std::move(paths), preserve_order);
    if (!prepared) {
        return domain::Result<domain::SermonDeck>::fail(prepared.error());
    }

    AssetImporter asset_importer{store_};
    std::vector<domain::SermonSlide> slides;
    slides.reserve(prepared->size());
    for (std::size_t index = 0; index != prepared->size(); ++index) {
        const auto asset = asset_importer.import_file((*prepared)[index]);
        if (!asset) {
            return domain::Result<domain::SermonDeck>::fail(asset.error());
        }
        slides.push_back(domain::SermonSlide{
            domain::CueId{"slide-" + std::to_string(index)},
            asset->id,
        });
    }

    domain::SermonDeck deck{std::move(id), std::move(title), std::move(slides)};
    return sermons_.save_deck(deck);
}

}  // namespace shekinah::storage
