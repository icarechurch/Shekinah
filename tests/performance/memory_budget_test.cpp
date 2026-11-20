#include <catch2/catch_test_macros.hpp>

#include "shekinah/media/image_cache.hpp"
#include "shekinah/media/memory_diagnostics.hpp"

using namespace shekinah::domain;
using namespace shekinah::media;

TEST_CASE("representative active still cache stays under its 36 MiB pool cap") {
    ImageCache cache{36ULL * 1024ULL * 1024ULL};
    for (int index = 0; index != 12; ++index) {
        cache.put(AssetId{"asset-" + std::to_string(index)}, ImageBytes{4ULL * 1024ULL * 1024ULL});
    }

    REQUIRE(cache.resident_bytes() <= 36ULL * 1024ULL * 1024ULL);
}

TEST_CASE("representative process working set stays below the 300 MB hard ceiling") {
    const auto working_set = MemoryDiagnostics::working_set_bytes();
    CAPTURE(working_set);
    REQUIRE(working_set < 300'000'000ULL);
}
