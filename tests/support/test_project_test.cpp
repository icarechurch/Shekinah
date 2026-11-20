#include <catch2/catch_test_macros.hpp>

#include "test_project.hpp"

TEST_CASE("TestProject creates an isolated writable project root") {
    const auto first = TestProject::create();
    const auto second = TestProject::create();
    REQUIRE(first.root() != second.root());
    REQUIRE(std::filesystem::is_directory(first.root()));
}
