#pragma once

#include "shekinah/application/live_slide_presenter.hpp"
#include "shekinah/domain/model.hpp"
#include "shekinah/ui/program_status_types.hpp"

#include <filesystem>
#include <optional>

namespace shekinah::storage {
class ProjectSession;
}

namespace shekinah::ui::main_window_internal {

struct LiveMediaSnapshot {
    application::LiveSlidePresentation presentation;
    std::optional<domain::AssetRecord> asset;
    std::filesystem::path asset_path;
};

class LiveMediaCoordinator final {
public:
    explicit LiveMediaCoordinator(storage::ProjectSession& session) noexcept;

    [[nodiscard]] bool ready() const noexcept;
    [[nodiscard]] std::optional<LiveMediaSnapshot> resolve(
        const domain::ProgramState& program) const;

private:
    storage::ProjectSession& session_;
};

[[nodiscard]] LiveSlideDetails make_live_details(
    const application::LiveSlidePresentation& presentation,
    const std::optional<domain::AssetRecord>& asset);

}  // namespace shekinah::ui::main_window_internal
