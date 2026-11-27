#pragma once

#include "shekinah/domain/model.hpp"
#include "shekinah/domain/result.hpp"

#include <optional>
#include <string>
#include <vector>

namespace shekinah::storage {
class ProjectSession;
}

namespace shekinah::ui {
class ContentPersistenceCoordinator;

struct SlideInspection final {
    domain::CueId cue;
    std::string section;
    std::vector<std::string> lines;
    domain::FitMode fit{domain::FitMode::cover};
};

class SlideInspectionCoordinator final {
public:
    SlideInspectionCoordinator(storage::ProjectSession& session,
                               ContentPersistenceCoordinator& persistence) noexcept;

    [[nodiscard]] std::optional<SlideInspection> inspect(const domain::ContentRef& content,
                                                         const domain::CueId& cue) const;
    [[nodiscard]] domain::Status save_song_slide(const domain::ContentRef& content,
                                                 const domain::CueId& cue,
                                                 std::string section,
                                                 std::vector<std::string> lines,
                                                 domain::FitMode fit) const;

private:
    storage::ProjectSession& session_;
    ContentPersistenceCoordinator& persistence_;
};

}  // namespace shekinah::ui
