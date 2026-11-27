#pragma once

#include "shekinah/application/cue_presenter.hpp"
#include "shekinah/domain/model.hpp"
#include "shekinah/domain/result.hpp"

namespace shekinah::storage {
class ProjectSession;
}

namespace shekinah::ui {

class CuePresentationCoordinator final {
public:
    explicit CuePresentationCoordinator(storage::ProjectSession& session) noexcept;

    [[nodiscard]] bool ready() const noexcept;
    domain::Result<application::CuePresentationSet> load(
        const domain::ContentRef& content) const;

private:
    storage::ProjectSession& session_;
};

}  // namespace shekinah::ui
