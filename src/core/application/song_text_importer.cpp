#include "shekinah/application/song_text_importer.hpp"

#include <cctype>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace shekinah::application {
namespace {

std::string_view trim(std::string_view value) {
    while (!value.empty() && std::isspace(static_cast<unsigned char>(value.front())) != 0) {
        value.remove_prefix(1);
    }
    while (!value.empty() && std::isspace(static_cast<unsigned char>(value.back())) != 0) {
        value.remove_suffix(1);
    }
    return value;
}

}  // namespace

domain::Result<domain::Song> SongTextImporter::parse(std::string_view text) {
    std::string title;
    std::string section;
    std::vector<domain::Slide> slides;
    std::vector<std::string> current_lines;
    std::size_t slide_ordinal = 0;
    std::size_t line_start = 0;

    const auto flush_slide = [&]() {
        if (current_lines.empty()) {
            return;
        }
        slides.push_back(domain::Slide{
            domain::CueId{"slide-" + std::to_string(slide_ordinal++)},
            section,
            std::move(current_lines),
            std::nullopt,
        });
        current_lines.clear();
    };

    while (line_start <= text.size()) {
        const auto line_end = text.find('\n', line_start);
        const auto end = line_end == std::string_view::npos ? text.size() : line_end;
        std::string_view line = text.substr(line_start, end - line_start);
        if (!line.empty() && line.back() == '\r') {
            line.remove_suffix(1);
        }
        const auto stripped = trim(line);

        if (stripped.starts_with("# ")) {
            if (title.empty()) {
                title = std::string{trim(stripped.substr(2))};
            }
        } else if (stripped.size() >= 2 && stripped.front() == '[' && stripped.back() == ']') {
            flush_slide();
            section = std::string{trim(stripped.substr(1, stripped.size() - 2))};
        } else if (stripped.empty()) {
            flush_slide();
        } else {
            current_lines.emplace_back(stripped);
        }

        if (line_end == std::string_view::npos) {
            break;
        }
        line_start = line_end + 1;
    }
    flush_slide();

    if (title.empty() && slides.empty()) {
        return domain::Result<domain::Song>::fail(
            {domain::ErrorCode::invalid_input, "Song text must contain a title or lyric content"});
    }
    if (title.empty()) {
        title = "Untitled Song";
    }

    return domain::Result<domain::Song>::ok(
        domain::Song{domain::SongId{"song-imported"}, std::move(title), std::nullopt, std::move(slides)});
}

}  // namespace shekinah::application
