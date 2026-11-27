#include "shekinah/application/song_reflow.hpp"

#include <algorithm>
#include <string>
#include <string_view>
#include <utility>

namespace shekinah::application {
namespace {

std::string_view trim_leading_line_breaks(std::string_view value) {
    while (!value.empty() && (value.front() == '\n' || value.front() == '\r')) {
        value.remove_prefix(1);
    }
    return value;
}

std::vector<std::string> split_lines(std::string_view value) {
    std::vector<std::string> lines;
    std::size_t start = 0;
    while (start <= value.size()) {
        const std::size_t end = value.find('\n', start);
        const std::size_t line_end = end == std::string_view::npos ? value.size() : end;
        std::string line{value.substr(start, line_end - start)};
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        lines.push_back(std::move(line));
        if (end == std::string_view::npos) {
            break;
        }
        start = end + 1;
    }
    return lines;
}

}  // namespace

std::vector<domain::Slide> SongReflowDocument::to_slides() const {
    std::vector<domain::Slide> slides;
    std::size_t start = 0;
    std::size_t ordinal = 0;

    for (const auto& slide_break : breaks_) {
        const std::size_t end = std::min(slide_break.utf8_offset, text_.size());
        const auto content = trim_leading_line_breaks(std::string_view{text_}.substr(start, end - start));
        slides.push_back(domain::Slide{
            domain::CueId{"slide-" + std::to_string(ordinal++)},
            slide_break.section_label,
            split_lines(content),
            std::nullopt,
        });
        start = end;
    }

    const auto content = trim_leading_line_breaks(std::string_view{text_}.substr(start));
    slides.push_back(domain::Slide{
        domain::CueId{"slide-" + std::to_string(ordinal)},
        {},
        split_lines(content),
        std::nullopt,
    });
    return slides;
}

}  // namespace shekinah::application
