#include <catch2/catch_test_macros.hpp>

#include "shekinah/media/image_cache.hpp"

using namespace shekinah::domain;
using namespace shekinah::media;

TEST_CASE("image cache evicts least recently used entries above its byte cap") {
    ImageCache cache{1024};
    cache.put(AssetId{"a"}, ImageBytes{700});
    cache.put(AssetId{"b"}, ImageBytes{700});

    REQUIRE(cache.resident_bytes() <= 1024);
    REQUIRE_FALSE(cache.contains(AssetId{"a"}));
    REQUIRE(cache.contains(AssetId{"b"}));
}

TEST_CASE("image cache rejects an entry larger than its complete budget") {
    ImageCache cache{1024};
    cache.put(AssetId{"large"}, ImageBytes{2048});

    REQUIRE(cache.resident_bytes() == 0);
    REQUIRE_FALSE(cache.contains(AssetId{"large"}));
}
