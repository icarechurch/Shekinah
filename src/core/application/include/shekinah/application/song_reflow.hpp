#pragma once

#include "shekinah/domain/model.hpp"
#include "shekinah/domain/result.hpp"

#include <cstddef>
#include <string>
#include <vector>

namespace shekinah::application {

struct SlideBreak {
    std::size_t utf8_offset{0};
    std::string section_label;
};

class SongReflowDocument {
public:
    SongReflowDocument(std::string text, std::vector<SlideBreak> breaks);

    [[nodiscard]] const std::string& text() const noexcept { return text_; }
    [[nodiscard]] const std::vector<SlideBreak>& breaks() const noexcept { return breaks_; }

    domain::Status insert_slide_break(std::size_t utf8_offset, std::string section_label = {});
    domain::Status remove_slide_break(std::size_t utf8_offset);
    [[nodiscard]] std::vector<domain::Slide> to_slides() const;

private:
    [[nodiscard]] bool is_utf8_boundary(std::size_t offset) const noexcept;

    std::string text_;
    std::vector<SlideBreak> breaks_;
};

}  // namespace shekinah::application
