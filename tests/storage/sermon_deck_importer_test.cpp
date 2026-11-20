#include <catch2/catch_test_macros.hpp>

#include <fstream>

#include "test_project.hpp"
#include "shekinah/storage/asset_importer.hpp"
#include "shekinah/storage/project_store.hpp"
#include "shekinah/storage/sermon_deck_importer.hpp"

using namespace shekinah::domain;
using namespace shekinah::storage;

TEST_CASE("sermon deck importer naturally orders PNG slides") {
    auto project = TestProject::create();
    auto store = ProjectStore::create_or_open(project.root() / "Sunday");
    REQUIRE(store);

    const auto slide_10 = project.root() / "Slide 10.png";
    const auto slide_2 = project.root() / "Slide 2.png";
    const auto slide_1 = project.root() / "slide 1.PNG";
    std::ofstream(slide_10, std::ios::binary) << "ten";
    std::ofstream(slide_2, std::ios::binary) << "two";
    std::ofstream(slide_1, std::ios::binary) << "one";

    SermonDeckImporter importer{*store};
    const auto deck = importer.import_pngs(
        SermonId{"sermon-a"}, "Sunday Sermon", {slide_10, slide_2, slide_1});

    REQUIRE(deck);
    REQUIRE(deck->slides.size() == 3);
    AssetImporter assets{*store};
    const auto expected_one = assets.import_file(slide_1);
    const auto expected_two = assets.import_file(slide_2);
    const auto expected_ten = assets.import_file(slide_10);
    REQUIRE(expected_one);
    REQUIRE(expected_two);
    REQUIRE(expected_ten);
    REQUIRE(deck->slides[0].image == expected_one->id);
    REQUIRE(deck->slides[1].image == expected_two->id);
    REQUIRE(deck->slides[2].image == expected_ten->id);
    REQUIRE(deck->slides[0].id.value == "slide-0");

}

TEST_CASE("sermon deck importer rejects duplicate names and non-PNG files") {
    auto project = TestProject::create();
    auto store = ProjectStore::create_or_open(project.root() / "Sunday");
    REQUIRE(store);

    const auto first = project.root() / "Slide 1.png";
    const auto duplicate = project.root() / "slide 1.PNG";
    const auto jpg = project.root() / "Slide 2.jpg";
    std::ofstream(first, std::ios::binary) << "one";
    std::ofstream(duplicate, std::ios::binary) << "duplicate";
    std::ofstream(jpg, std::ios::binary) << "jpg";

    SermonDeckImporter importer{*store};
    const auto duplicate_result = importer.import_pngs(
        SermonId{"sermon-a"}, "Sunday Sermon", {first, duplicate});
    REQUIRE_FALSE(duplicate_result);
    REQUIRE(duplicate_result.error().code == ErrorCode::invalid_input);

    const auto extension_result = importer.import_pngs(
        SermonId{"sermon-b"}, "Sunday Sermon", {jpg});
    REQUIRE_FALSE(extension_result);
    REQUIRE(extension_result.error().code == ErrorCode::unsupported_media);
}
