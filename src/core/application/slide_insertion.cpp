#include "shekinah/application/slide_insertion.hpp"

#include <algorithm>
#include <string>
#include <utility>

namespace shekinah::application {
namespace {

template <typename SlideRange>
domain::CueId allocate_unique_cue_id(const SlideRange& slides, const std::string& prefix) {
    std::size_t suffix = slides.size();
    for (;;) {
        domain::CueId candidate{prefix + std::to_string(suffix)};
        const bool exists = std::any_of(slides.begin(), slides.end(), [&](const auto& slide) {
            return slide.id == candidate;
        });
        if (!exists) {
            return candidate;
        }
        ++suffix;
    }
}

}  // namespace

domain::Result<domain::CueId> insert_asset_slide(domain::Song& song,
                                                 std::size_t index,
                                                 domain::BackgroundRef background) {
    if (index > song.slides.size()) {
        return domain::Result<domain::CueId>::fail(
            {domain::ErrorCode::invalid_input, "Slide insertion index is out of range"});
    }
    auto cue_id = allocate_unique_cue_id(song.slides, song.id.value + "-media-");
    song.slides.insert(song.slides.begin() + static_cast<std::ptrdiff_t>(index),
                       domain::Slide{cue_id, "Media", {}, std::move(background)});
    return domain::Result<domain::CueId>::ok(std::move(cue_id));
}

domain::Result<domain::CueId> insert_asset_slide(domain::SermonDeck& deck,
                                                 std::size_t index,
                                                 domain::AssetId asset) {
    if (index > deck.slides.size()) {
        return domain::Result<domain::CueId>::fail(
            {domain::ErrorCode::invalid_input, "Slide insertion index is out of range"});
    }
    auto cue_id = allocate_unique_cue_id(deck.slides, deck.id.value + "-slide-");
    deck.slides.insert(deck.slides.begin() + static_cast<std::ptrdiff_t>(index),
                       domain::SermonSlide{cue_id, std::move(asset)});
    return domain::Result<domain::CueId>::ok(std::move(cue_id));
}

}  // namespace shekinah::application
