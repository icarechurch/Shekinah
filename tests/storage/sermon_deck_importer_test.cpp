#include <catch2/catch_test_macros.hpp>

#include <array>
#include <fstream>

#include "test_project.hpp"
#include "shekinah/storage/asset_importer.hpp"
#include "shekinah/storage/content_repository.hpp"
#include "shekinah/storage/project_store.hpp"
#include "shekinah/storage/sermon_deck_importer.hpp"

using namespace shekinah::domain;
using namespace shekinah::storage;

TEST_CASE("sermon deck importer naturally orders sermon files") {
    auto project = TestProject::create();
    auto store = ProjectStore::create_or_open(project.root() / "Sunday");
    REQUIRE(store);

    const auto slide_10 = project.root() / "Slide 10.png";
    const auto slide_2 = project.root() / "Slide 2.png";
    const auto slide_1 = project.root() / "slide 1.PNG";
    std::ofstream(slide_10, std::ios::binary) << "ten";
    std::ofstream(slide_2, std::ios::binary) << "two";
    std::ofstream(slide_1, std::ios::binary) << "one";

    SqliteContentRepository repository{*store};
    SermonDeckImporter importer{*store, repository};
    const auto deck = importer.import_files(
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
    const auto loaded = SqliteContentRepository{*store}.load_deck(SermonId{"sermon-a"});
    REQUIRE(loaded);
    REQUIRE(loaded->title == "Sunday Sermon");
    REQUIRE(loaded->slides.size() == 3);
    REQUIRE(loaded->slides[0].image == expected_one->id);
    REQUIRE(deck->slides[0].id.value == "slide-0");

}

TEST_CASE("sermon deck importer accepts mixed supported media") {
    auto project = TestProject::create();
    auto store = ProjectStore::create_or_open(project.root() / "Sunday");
    REQUIRE(store);

    const auto jpg = project.root() / "Slide 1.jpg";
    const auto gif = project.root() / "Slide 2.GIF";
    const auto mp4 = project.root() / "Slide 3.MP4";
    std::ofstream(jpg, std::ios::binary) << "jpg-bytes";
    std::ofstream(gif, std::ios::binary) << "gif-bytes";
    std::ofstream(mp4, std::ios::binary) << "mp4-bytes";

    SqliteContentRepository repository{*store};
    SermonDeckImporter importer{*store, repository};
    const auto deck = importer.import_files(
        SermonId{"sermon-media"}, "Media Sermon", {jpg, gif, mp4}, true);

    REQUIRE(deck);
    REQUIRE(deck->slides.size() == 3);

    const auto first = repository.load_asset(deck->slides[0].image);
    const auto second = repository.load_asset(deck->slides[1].image);
    const auto third = repository.load_asset(deck->slides[2].image);
    REQUIRE(first);
    REQUIRE(second);
    REQUIRE(third);
    REQUIRE(first->kind == AssetKind::image);
    REQUIRE(second->kind == AssetKind::image);
    REQUIRE(third->kind == AssetKind::video);
}

TEST_CASE("sermon deck importer accepts every supported asset media extension") {
    auto project = TestProject::create();
    auto store = ProjectStore::create_or_open(project.root() / "Sunday");
    REQUIRE(store);

    constexpr std::array extensions{
        ".png", ".jpg", ".jpeg", ".webp", ".bmp", ".gif", ".tif", ".tiff", ".mp4"};
    std::vector<std::filesystem::path> paths;
    paths.reserve(extensions.size());
    for (std::size_t index = 0; index != extensions.size(); ++index) {
        const auto path = project.root() / ("Slide " + std::to_string(index + 1) + extensions[index]);
        std::ofstream(path, std::ios::binary) << "media-" << index;
        paths.push_back(path);
    }

    SqliteContentRepository repository{*store};
    SermonDeckImporter importer{*store, repository};
    const auto deck = importer.import_files(
        SermonId{"sermon-all-media"}, "All Media Sermon", std::move(paths), true);

    REQUIRE(deck);
    REQUIRE(deck->slides.size() == extensions.size());
}

TEST_CASE("sermon deck importer rejects duplicate names and unsupported media") {
    auto project = TestProject::create();
    auto store = ProjectStore::create_or_open(project.root() / "Sunday");
    REQUIRE(store);

    const auto first = project.root() / "Slide 1.png";
    const auto duplicate = project.root() / "slide 1.PNG";
    const auto unsupported = project.root() / "Slide 2.txt";
    std::ofstream(first, std::ios::binary) << "one";
    std::ofstream(duplicate, std::ios::binary) << "duplicate";
    std::ofstream(unsupported, std::ios::binary) << "text";

    SqliteContentRepository repository{*store};
    SermonDeckImporter importer{*store, repository};
    const auto duplicate_result = importer.import_files(
        SermonId{"sermon-a"}, "Sunday Sermon", {first, duplicate});
    REQUIRE_FALSE(duplicate_result);
    REQUIRE(duplicate_result.error().code == ErrorCode::invalid_input);

    const auto extension_result = importer.import_files(
        SermonId{"sermon-b"}, "Sunday Sermon", {unsupported});
    REQUIRE_FALSE(extension_result);
    REQUIRE(extension_result.error().code == ErrorCode::unsupported_media);
}

TEST_CASE("sermon deck importer preserves the operator-defined order when requested") {
    auto project = TestProject::create();
    auto store = ProjectStore::create_or_open(project.root() / "Sunday");
    REQUIRE(store);

    const auto third = project.root() / "Slide 3.png";
    const auto first = project.root() / "Slide 1.png";
    const auto second = project.root() / "Slide 2.png";
    std::ofstream(third, std::ios::binary) << "three";
    std::ofstream(first, std::ios::binary) << "one";
    std::ofstream(second, std::ios::binary) << "two";

    SqliteContentRepository repository{*store};
    SermonDeckImporter importer{*store, repository};
    const auto deck = importer.import_files(
        SermonId{"sermon-ordered"}, "Ordered Sermon", {third, first, second}, true);

    REQUIRE(deck);
    AssetImporter assets{*store};
    const auto expected_third = assets.import_file(third);
    const auto expected_first = assets.import_file(first);
    const auto expected_second = assets.import_file(second);
    REQUIRE(expected_third);
    REQUIRE(expected_first);
    REQUIRE(expected_second);
    REQUIRE(deck->slides[0].image == expected_third->id);
    REQUIRE(deck->slides[1].image == expected_first->id);
    REQUIRE(deck->slides[2].image == expected_second->id);
}
