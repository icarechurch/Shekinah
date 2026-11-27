#include "shekinah/application/song_reflow.hpp"

#include <algorithm>
#include <string>
#include <utility>

namespace shekinah::application {
namespace {

domain::Error invalid_break(std::string message) {
    return {domain::ErrorCode::invalid_input, std::move(message)};
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

}  // namespace shekinah::application
