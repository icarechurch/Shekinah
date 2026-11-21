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

TEST_CASE("deduplicated imports return the canonical stored path") {
    auto project = TestProject::create();
    auto store = ProjectStore::create_or_open(project.root() / "Sunday");
    REQUIRE(store);
    const auto first_path = project.root() / "first.PNG";
    const auto second_path = project.root() / "second.jpg";
    std::ofstream(first_path, std::ios::binary) << "same-bytes";
    std::ofstream(second_path, std::ios::binary) << "same-bytes";

    AssetImporter importer{*store};
    const auto first = importer.import_file(first_path);
    const auto second = importer.import_file(second_path);

    REQUIRE(first);
    REQUIRE(second);
    REQUIRE(second->id == first->id);
    REQUIRE(second->relative_path == first->relative_path);
    REQUIRE(std::filesystem::exists(project.root() / "Sunday" / second->relative_path));
}

TEST_CASE("asset importer accepts common ProContent image extensions") {
    auto project = TestProject::create();
    auto store = ProjectStore::create_or_open(project.root() / "Sunday");
    REQUIRE(store);
    const auto webp_path = project.root() / "background.webp";
    const auto bmp_path = project.root() / "texture.bmp";
    std::ofstream(webp_path, std::ios::binary) << "webp-bytes";
    std::ofstream(bmp_path, std::ios::binary) << "bmp-bytes";

    AssetImporter importer{*store};
    const auto webp = importer.import_file(webp_path);
    const auto bmp = importer.import_file(bmp_path);

    REQUIRE(webp);
    REQUIRE(bmp);
    REQUIRE(webp->kind == AssetKind::image);
    REQUIRE(bmp->kind == AssetKind::image);
}
