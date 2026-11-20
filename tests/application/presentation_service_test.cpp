#include <catch2/catch_test_macros.hpp>

#include "shekinah/application/presentation_service.hpp"
#include "shekinah/application/service_order_service.hpp"

using namespace shekinah::application;
using namespace shekinah::domain;

namespace {

Song make_song(std::string id) {
    return Song{
        SongId{std::move(id)},
        "Goodness",
        BackgroundRef{AssetId{"default-video"}, FitMode::cover},
        {
            Slide{CueId{"verse-1"}, "Verse 1", {"One"}, std::nullopt},
            Slide{CueId{"chorus-1"}, "Chorus", {"Two"}, BackgroundRef{AssetId{"slide-image"}, FitMode::contain}},
            Slide{CueId{"bridge-1"}, "Bridge", {"Three"}, std::nullopt},
        },
    };
}

}  // namespace

TEST_CASE("selecting a service item never changes the current program") {
    PresentationService service;
    service.register_song(make_song("song-a"));
    service.register_song(make_song("song-b"));
    ServiceOrderService order{ServiceOrderId{"sunday"}};
    const auto first = order.append(ContentRef::song("song-a"));
    const auto second = order.append(ContentRef::song("song-b"));
    REQUIRE(first);
    REQUIRE(second);
    REQUIRE(service.set_service_order(order.items()));
    REQUIRE(service.take_live(first->id, CueId{"chorus-1"}));

    REQUIRE(service.select_service_item(second->id));
    REQUIRE(service.program().live_cue->cue.value == "chorus-1");
}

TEST_CASE("direct cue jumps and global navigation operate on the selected item") {
    PresentationService service;
    service.register_song(make_song("song-a"));
    ServiceOrderService order{ServiceOrderId{"sunday"}};
    const auto item = order.append(ContentRef::song("song-a"));
    REQUIRE(item);
    REQUIRE(service.set_service_order(order.items()));

    REQUIRE(service.take_live(item->id, CueId{"bridge-1"}));
    REQUIRE(service.previous());
    REQUIRE(service.program().live_cue->cue.value == "chorus-1");
    REQUIRE(service.next());
    REQUIRE(service.program().live_cue->cue.value == "bridge-1");
    REQUIRE(service.take_live(item->id, CueId{"verse-1"}));
    REQUIRE_FALSE(service.previous());
    REQUIRE(service.program().live_cue->cue.value == "verse-1");
}

TEST_CASE("background resolution prefers slide override then Song default") {
    PresentationService service;
    service.register_song(make_song("song-a"));
    const auto override_background = service.resolve_background(
        ContentRef::song("song-a"), CueId{"chorus-1"});
    REQUIRE(override_background);
    REQUIRE(override_background->value().asset.value == "slide-image");

    const auto default_background = service.resolve_background(
        ContentRef::song("song-a"), CueId{"verse-1"});
    REQUIRE(default_background);
    REQUIRE(default_background->value().asset.value == "default-video");
}

TEST_CASE("clear and black update Program without introducing a preview state") {
    PresentationService service;
    service.register_song(make_song("song-a"));
    ServiceOrderService order{ServiceOrderId{"sunday"}};
    const auto item = order.append(ContentRef::song("song-a"));
    REQUIRE(item);
    REQUIRE(service.set_service_order(order.items()));
    REQUIRE(service.take_live(item->id, CueId{"verse-1"}));

    service.clear();
    REQUIRE(service.program().mode == ProgramMode::clear);
    service.black();
    REQUIRE(service.program().mode == ProgramMode::black);
}
