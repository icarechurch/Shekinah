#pragma once

#include "shekinah/domain/model.hpp"

namespace shekinah::domain {

class PresentationEngine {
public:
    void select_content(ContentRef content);
    void take_live(CueRef cue);
    void clear() noexcept;
    void black() noexcept;

    [[nodiscard]] const ProgramState& program() const noexcept { return program_; }
    [[nodiscard]] const std::optional<ContentRef>& selected_content() const noexcept {
        return selected_content_;
    }

private:
    std::optional<ContentRef> selected_content_;
    ProgramState program_{};
};

}  // namespace shekinah::domain
