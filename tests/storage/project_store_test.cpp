#include <catch2/catch_test_macros.hpp>

#include "test_project.hpp"
#include "shekinah/storage/project_store.hpp"

using namespace shekinah::domain;
using namespace shekinah::storage;

TEST_CASE("opening a project creates a migrated SQLite database and assets directory") {
    auto project = TestProject::create();
    auto store = ProjectStore::create_or_open(project.root() / "Sunday");

    REQUIRE(store);
    REQUIRE(std::filesystem::exists(project.root() / "Sunday" / "shekinah.sqlite"));
    REQUIRE(std::filesystem::is_directory(project.root() / "Sunday" / "assets"));
    REQUIRE(store->schema_version() == 1);
    REQUIRE(store->table_exists("service_order_items"));
}

TEST_CASE("a failed transaction rolls back its database changes") {
    auto project = TestProject::create();
    auto store = ProjectStore::create_or_open(project.root() / "Sunday");
    REQUIRE(store);

    const auto status = store->transaction([](sqlite3* db) {
        const auto rc = sqlite3_exec(db, "CREATE TABLE transient_value(value INTEGER);", nullptr, nullptr, nullptr);
        if (rc != SQLITE_OK) {
            return Status::fail(Error{ErrorCode::storage_failure, "create failed"});
        }
        return Status::fail(Error{ErrorCode::storage_failure, "forced rollback"});
    });

    REQUIRE_FALSE(status);
    REQUIRE_FALSE(store->table_exists("transient_value"));
}
