#include "shekinah/application/song_reflow.hpp"

#include <algorithm>
#include <string_view>

namespace shekinah::application {
namespace {

domain::Error invalid_break(std::string message) {
    return {domain::ErrorCode::invalid_input, std::move(message)};
}

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

SongReflowDocument::SongReflowDocument(std::string text, std::vector<SlideBreak> breaks)
    : text_(std::move(text)), breaks_(std::move(breaks)) {
    std::sort(breaks_.begin(), breaks_.end(), [](const SlideBreak& left, const SlideBreak& right) {
        return left.utf8_offset < right.utf8_offset;
    });
}

bool SongReflowDocument::is_utf8_boundary(std::size_t offset) const noexcept {
    if (offset > text_.size()) {
        return false;
    }
    if (offset == 0 || offset == text_.size()) {
        return true;
    }
    const auto byte = static_cast<unsigned char>(text_[offset]);
    return (byte & 0xC0U) != 0x80U;
}

domain::Status SongReflowDocument::insert_slide_break(std::size_t utf8_offset, std::string section_label) {
    if (!is_utf8_boundary(utf8_offset) || utf8_offset == 0 || utf8_offset == text_.size()) {
        return domain::Status::fail(invalid_break("A slide break must be inside the song text at a UTF-8 boundary"));
    }

    const auto existing = std::lower_bound(
        breaks_.begin(), breaks_.end(), utf8_offset, [](const SlideBreak& item, std::size_t offset) {
            return item.utf8_offset < offset;
        });
    if (existing != breaks_.end() && existing->utf8_offset == utf8_offset) {
        return domain::Status::fail(invalid_break("A slide break already exists at this offset"));
    }

    breaks_.insert(existing, SlideBreak{utf8_offset, std::move(section_label)});
    return domain::success();
}

domain::Status SongReflowDocument::remove_slide_break(std::size_t utf8_offset) {
    const auto existing = std::lower_bound(
        breaks_.begin(), breaks_.end(), utf8_offset, [](const SlideBreak& item, std::size_t offset) {
            return item.utf8_offset < offset;
        });
    if (existing == breaks_.end() || existing->utf8_offset != utf8_offset) {
        return domain::Status::fail({domain::ErrorCode::not_found, "No slide break exists at this offset"});
    }

    breaks_.erase(existing);
    return domain::success();
}

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
