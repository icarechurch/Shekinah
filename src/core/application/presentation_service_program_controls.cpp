#include "shekinah/application/presentation_service.hpp"

namespace shekinah::application {

void PresentationService::clear() noexcept {
    program_ = domain::ProgramState{domain::ProgramMode::clear, std::nullopt};
}

void PresentationService::black() noexcept {
    program_ = domain::ProgramState{domain::ProgramMode::black, std::nullopt};
}

}  // namespace shekinah::application
