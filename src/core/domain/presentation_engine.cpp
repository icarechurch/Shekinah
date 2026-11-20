#include "shekinah/domain/presentation_engine.hpp"

#include <utility>

namespace shekinah::domain {

void PresentationEngine::select_content(ContentRef content) {
    selected_content_ = std::move(content);
}

void PresentationEngine::take_live(CueRef cue) {
    program_ = ProgramState{ProgramMode::cue, std::move(cue)};
}

void PresentationEngine::clear() noexcept {
    program_ = ProgramState{ProgramMode::clear, std::nullopt};
}

void PresentationEngine::black() noexcept {
    program_ = ProgramState{ProgramMode::black, std::nullopt};
}

}  // namespace shekinah::domain
