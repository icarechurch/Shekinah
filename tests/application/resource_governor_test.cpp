#include <catch2/catch_test_macros.hpp>

#include "shekinah/application/resource_governor.hpp"

using namespace shekinah::application;

namespace {

class FakePools final : public ResourcePools {
public:
    void purge_non_live_caches() override { purged = true; }
    void cancel_queued_work() override { cancelled = true; }
    void release_non_active_media() override { released = true; }

    bool purged{false};
    bool cancelled{false};
    bool released{false};
};

}  // namespace

TEST_CASE("resource governor purges caches before reaching the hard limit") {
    FakePools pools;
    ResourceGovernor governor{pools};

    governor.on_working_set_sample(220'000'000);
    REQUIRE(pools.purged);
    REQUIRE(governor.may_load_live_media());

    governor.on_working_set_sample(250'000'000);
    REQUIRE(pools.cancelled);
    REQUIRE(pools.released);
    REQUIRE(governor.may_load_live_media());

    governor.on_working_set_sample(270'000'000);
    REQUIRE_FALSE(governor.may_load_live_media());
    REQUIRE(governor.last_sample_bytes() == 270'000'000);
}

TEST_CASE("resource governor reopens live-media admission after pressure falls") {
    FakePools pools;
    ResourceGovernor governor{pools};
    governor.on_working_set_sample(275'000'000);
    REQUIRE_FALSE(governor.may_load_live_media());
    governor.on_working_set_sample(180'000'000);
    REQUIRE(governor.may_load_live_media());
}
