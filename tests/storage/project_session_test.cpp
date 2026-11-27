#include <catch2/catch_test_macros.hpp>

#include "test_project.hpp"
#include "shekinah/storage/project_session.hpp"

#include <fstream>

using namespace shekinah::storage;

TEST_CASE("project session opens a project and exposes typed feature boundaries") {
    auto project = TestProject::create();
    ProjectSession session;

    REQUIRE_FALSE(session.is_open());
    REQUIRE(session.open(project.root() / "Sunday"));
    REQUIRE(session.is_open());
    REQUIRE(session.root() == project.root() / "Sunday");
    REQUIRE(session.songs() != nullptr);
    REQUIRE(session.sermons() != nullptr);
    REQUIRE(session.assets() != nullptr);
    REQUIRE(session.service_orders() != nullptr);
    REQUIRE(session.content_archive() != nullptr);
    REQUIRE(session.asset_importer() != nullptr);
    REQUIRE(session.sermon_importer() != nullptr);
}

TEST_CASE("project session close releases the project boundary") {
    auto project = TestProject::create();
    ProjectSession session;
    REQUIRE(session.open(project.root() / "Sunday"));

    session.close();

    REQUIRE_FALSE(session.is_open());
    REQUIRE(session.root().empty());
    REQUIRE(session.songs() == nullptr);
    REQUIRE(session.sermons() == nullptr);
    REQUIRE(session.assets() == nullptr);
    REQUIRE(session.service_orders() == nullptr);
    REQUIRE(session.content_archive() == nullptr);
    REQUIRE(session.asset_importer() == nullptr);
    REQUIRE(session.sermon_importer() == nullptr);
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
    REQUIRE(session.songs() == nullptr);
    REQUIRE(session.sermons() == nullptr);
    REQUIRE(session.assets() == nullptr);
    REQUIRE(session.service_orders() == nullptr);
    REQUIRE(session.content_archive() == nullptr);
    REQUIRE(session.asset_importer() == nullptr);
    REQUIRE(session.sermon_importer() == nullptr);
}
