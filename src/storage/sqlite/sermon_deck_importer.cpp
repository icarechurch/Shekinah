#include "shekinah/storage/sermon_deck_importer.hpp"

#include "shekinah/storage/asset_importer.hpp"
#include "shekinah/storage/content_repository.hpp"

#include <algorithm>
#include <cctype>
#include <string>
#include <utility>

namespace shekinah::storage {
namespace {

std::string normalized_filename(const std::filesystem::path& path) {
    auto value = path.filename().string();
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char character) {
        return static_cast<char>(std::tolower(character));
    });
    return value;
}

bool natural_less(const std::string& left, const std::string& right) {
    std::size_t left_index = 0;
    std::size_t right_index = 0;
    while (left_index < left.size() && right_index < right.size()) {
        const auto left_digit = std::isdigit(static_cast<unsigned char>(left[left_index])) != 0;
        const auto right_digit = std::isdigit(static_cast<unsigned char>(right[right_index])) != 0;
        if (left_digit && right_digit) {
            const auto left_start = left_index;
            const auto right_start = right_index;
            while (left_index < left.size() && left[left_index] == '0') {
                ++left_index;
            }
            while (right_index < right.size() && right[right_index] == '0') {
                ++right_index;
            }
            const auto left_digits_start = left_index;
            const auto right_digits_start = right_index;
            while (left_index < left.size() && std::isdigit(static_cast<unsigned char>(left[left_index])) != 0) {
                ++left_index;
            }
            while (right_index < right.size() && std::isdigit(static_cast<unsigned char>(right[right_index])) != 0) {
                ++right_index;
            }
            const auto left_length = left_index - left_digits_start;
            const auto right_length = right_index - right_digits_start;
            if (left_length != right_length) {
                return left_length < right_length;
            }
            const auto left_number = left.substr(left_digits_start, left_length);
            const auto right_number = right.substr(right_digits_start, right_length);
            if (left_number != right_number) {
                return left_number < right_number;
            }
            const auto left_original_length = left_index - left_start;
            const auto right_original_length = right_index - right_start;
            if (left_original_length != right_original_length) {
                return left_original_length < right_original_length;
            }
            continue;
        }
        if (left[left_index] != right[right_index]) {
            return left[left_index] < right[right_index];
        }
        ++left_index;
        ++right_index;
    }
    return left.size() < right.size();
}

}  // namespace

domain::Result<domain::SermonDeck> SermonDeckImporter::import_pngs(
    domain::SermonId id,
    std::string title,
    std::vector<std::filesystem::path> paths,
    bool preserve_order) {
    if (id.empty() || title.empty() || paths.empty()) {
        return domain::Result<domain::SermonDeck>::fail(
            {domain::ErrorCode::invalid_input, "A Sermon deck needs an id, title, and at least one PNG"});
    }

    struct Candidate {
        std::filesystem::path path;
        std::string normalized_name;
    };
    std::vector<Candidate> candidates;
    candidates.reserve(paths.size());
    for (auto& path : paths) {
        const auto normalized_name = normalized_filename(path);
        if (path.extension().string().empty() || normalized_name.ends_with(".png") == false) {
            return domain::Result<domain::SermonDeck>::fail(
                {domain::ErrorCode::unsupported_media, "Sermon decks accept PNG slides only"});
        }
        candidates.push_back(Candidate{std::move(path), normalized_name});
    }

    if (!preserve_order) {
        std::sort(candidates.begin(), candidates.end(), [](const Candidate& left, const Candidate& right) {
            if (left.normalized_name == right.normalized_name) {
                return left.path.string() < right.path.string();
            }
            return natural_less(left.normalized_name, right.normalized_name);
        });
    }
    for (std::size_t left = 0; left < candidates.size(); ++left) {
        for (std::size_t right = left + 1; right < candidates.size(); ++right) {
            if (candidates[left].normalized_name == candidates[right].normalized_name) {
                return domain::Result<domain::SermonDeck>::fail(
                    {domain::ErrorCode::invalid_input, "Sermon slide filenames must be unique"});
            }
        }
    }

    AssetImporter asset_importer{store_};
    std::vector<domain::SermonSlide> slides;
    slides.reserve(candidates.size());
    for (std::size_t index = 0; index != candidates.size(); ++index) {
        const auto asset = asset_importer.import_file(candidates[index].path);
        if (!asset) {
            return domain::Result<domain::SermonDeck>::fail(asset.error());
        }
        slides.push_back(domain::SermonSlide{
            domain::CueId{"slide-" + std::to_string(index)},
            asset->id,
        });
    }

    domain::SermonDeck deck{std::move(id), std::move(title), std::move(slides)};
    return SqliteContentRepository{store_}.save_deck(deck);
}

}  // namespace shekinah::storage
