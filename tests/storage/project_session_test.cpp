#include <catch2/catch_test_macros.hpp>

#include "test_project.hpp"
#include "shekinah/storage/project_session.hpp"

#include <fstream>

using namespace shekinah::storage;

TEST_CASE("project session opens a project and exposes its owned storage boundary") {
    auto project = TestProject::create();
    ProjectSession session;

    REQUIRE_FALSE(session.is_open());
    REQUIRE(session.open(project.root() / "Sunday"));
    REQUIRE(session.is_open());
    REQUIRE(session.root() == project.root() / "Sunday");
    REQUIRE(session.store() != nullptr);
    REQUIRE(session.repository() != nullptr);
}

TEST_CASE("project session close releases the project boundary") {
    auto project = TestProject::create();
    ProjectSession session;
    REQUIRE(session.open(project.root() / "Sunday"));

    session.close();

    REQUIRE_FALSE(session.is_open());
    REQUIRE(session.root().empty());
    REQUIRE(session.store() == nullptr);
    REQUIRE(session.repository() == nullptr);
}

TEST_CASE("failed project open leaves the session closed") {
    auto project = TestProject::create();
    ProjectSession session;
    REQUIRE(session.open(project.root() / "Sunday"));

    const auto invalid_root = project.root() / "not-a-directory";
    std::ofstream{invalid_root} << "not a directory";

    REQUIRE_FALSE(session.open(invalid_root));
    REQUIRE_FALSE(session.is_open());
    REQUIRE(session.root().empty());
    REQUIRE(session.store() == nullptr);
    REQUIRE(session.repository() == nullptr);
}
