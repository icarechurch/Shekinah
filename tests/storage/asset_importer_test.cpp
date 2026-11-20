#include <catch2/catch_test_macros.hpp>

#include <fstream>

#include "test_project.hpp"
#include "shekinah/storage/asset_importer.hpp"
#include "shekinah/storage/project_store.hpp"

using namespace shekinah::domain;
using namespace shekinah::storage;

TEST_CASE("asset importer copies supported files and deduplicates identical bytes") {
    auto project = TestProject::create();
    auto store = ProjectStore::create_or_open(project.root() / "Sunday");
    REQUIRE(store);

    const auto first_path = project.root() / "first.png";
    const auto second_path = project.root() / "second.png";
    std::ofstream(first_path, std::ios::binary) << "image-bytes";
    std::ofstream(second_path, std::ios::binary) << "image-bytes";

    AssetImporter importer{*store};
    const auto first = importer.import_file(first_path);
    const auto second = importer.import_file(second_path);

    REQUIRE(first);
    REQUIRE(second);
    REQUIRE(first->id == second->id);
    REQUIRE(std::filesystem::exists(project.root() / "Sunday" / first->relative_path));
}

TEST_CASE("asset importer rejects unsupported extensions") {
    auto project = TestProject::create();
    auto store = ProjectStore::create_or_open(project.root() / "Sunday");
    REQUIRE(store);
    const auto source = project.root() / "notes.txt";
    std::ofstream(source) << "not media";

    AssetImporter importer{*store};
    const auto result = importer.import_file(source);

    REQUIRE_FALSE(result);
    REQUIRE(result.error().code == ErrorCode::unsupported_media);
}
