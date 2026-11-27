#include "slide_inspection_coordinator.hpp"

#include "content_persistence_coordinator.hpp"
#include "shekinah/storage/project_session.hpp"
#include "shekinah/storage/sermon_repository.hpp"
#include "shekinah/storage/song_repository.hpp"

#include <algorithm>
#include <utility>
#include <variant>

namespace shekinah::ui {
namespace {

domain::Status inspection_unavailable() {
    return domain::Status::fail(
        {domain::ErrorCode::storage_failure, "Slide inspection is unavailable"});
}

}  // namespace

SlideInspectionCoordinator::SlideInspectionCoordinator(
    storage::ProjectSession& session, ContentPersistenceCoordinator& persistence) noexcept
    : session_(session), persistence_(persistence) {}

std::optional<SlideInspection> SlideInspectionCoordinator::inspect(
    const domain::ContentRef& content, const domain::CueId& cue) const {
    if (!session_.songs() || !session_.sermons()) {
        return std::nullopt;
    }
    if (content.kind == domain::ContentKind::song) {
        const auto song = session_.songs()->load_song(std::get<domain::SongId>(content.id));
        if (!song) {
            return std::nullopt;
        }
        const auto slide = std::find_if(song->slides.begin(), song->slides.end(), [&](const auto& candidate) {
            return candidate.id == cue;
        });
        if (slide == song->slides.end()) {
            return std::nullopt;
        }
        return SlideInspection{slide->id,
                               slide->section,
                               slide->lines,
                               slide->background.has_value() ? slide->background->fit
                                                             : domain::FitMode::cover};
    }

    const auto sermon = session_.sermons()->load_deck(std::get<domain::SermonId>(content.id));
    if (!sermon) {
        return std::nullopt;
    }
    const auto slide = std::find_if(sermon->slides.begin(), sermon->slides.end(), [&](const auto& candidate) {
        return candidate.id == cue;
    });
    if (slide == sermon->slides.end()) {
        return std::nullopt;
    }
    return SlideInspection{slide->id, "Sermon slide", {slide->image.value}, domain::FitMode::cover};
}

domain::Status SlideInspectionCoordinator::save_song_slide(
    const domain::ContentRef& content,
    const domain::CueId& cue,
    std::string section,
    std::vector<std::string> lines,
    domain::FitMode fit) const {
    if (!session_.songs() || content.kind != domain::ContentKind::song) {
        return inspection_unavailable();
    }
    auto song = session_.songs()->load_song(std::get<domain::SongId>(content.id));
    if (!song) {
        return inspection_unavailable();
    }
    const auto slide = std::find_if(song->slides.begin(), song->slides.end(), [&](const auto& candidate) {
        return candidate.id == cue;
    });
    if (slide == song->slides.end()) {
        return inspection_unavailable();
    }
    slide->section = std::move(section);
    slide->lines = std::move(lines);
    if (slide->background.has_value()) {
        slide->background->fit = fit;
    }
    const auto saved = persistence_.save_song(*song);
    return saved ? domain::success() : domain::Status::fail(saved.error());
}

}  // namespace shekinah::ui
