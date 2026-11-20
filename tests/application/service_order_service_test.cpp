#include <catch2/catch_test_macros.hpp>

#include "shekinah/application/service_order_service.hpp"

using namespace shekinah::application;
using namespace shekinah::domain;

TEST_CASE("a service order accepts repeated references without duplicating a song") {
    ServiceOrderService service{ServiceOrderId{"sunday"}};

    for (int index = 0; index != 500; ++index) {
        REQUIRE(service.append(ContentRef::song("song-a")));
    }

    REQUIRE(service.items().size() == 500);
    REQUIRE(service.items().front().content == ContentRef::song("song-a"));
    REQUIRE(service.items().back().ordinal == 499);
}

TEST_CASE("service order removal leaves the reusable content reference intact") {
    ServiceOrderService service{ServiceOrderId{"sunday"}};
    const auto appended = service.append(ContentRef::sermon("sermon-a"));
    REQUIRE(appended);

    REQUIRE(service.remove(appended->id));
    REQUIRE(service.items().empty());
}

TEST_CASE("service order move keeps repeated content references and repairs ordinals") {
    ServiceOrderService service{ServiceOrderId{"sunday"}};
    REQUIRE(service.append(ContentRef::song("song-a")));
    REQUIRE(service.append(ContentRef::sermon("sermon-a")));
    REQUIRE(service.append(ContentRef::song("song-a")));
    const auto moved_id = service.items().back().id;

    REQUIRE(service.move(moved_id, 0));
    REQUIRE(service.items().at(0).content == ContentRef::song("song-a"));
    REQUIRE(service.items().at(1).content == ContentRef::song("song-a"));
    REQUIRE(service.items().at(2).content == ContentRef::sermon("sermon-a"));
    REQUIRE(service.items().at(0).ordinal == 0);
    REQUIRE(service.items().at(1).ordinal == 1);
    REQUIRE(service.items().at(2).ordinal == 2);
}
