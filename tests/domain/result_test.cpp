#include <catch2/catch_test_macros.hpp>

#include "shekinah/domain/result.hpp"

using namespace shekinah::domain;

TEST_CASE("Result carries either a value or a structured error") {
    auto success = Result<int>::ok(42);
    REQUIRE(success);
    REQUIRE(success.value() == 42);

    auto failure = Result<int>::fail(Error{ErrorCode::invalid_input, "bad input"});
    REQUIRE_FALSE(failure);
    REQUIRE(failure.error().code == ErrorCode::invalid_input);
}
