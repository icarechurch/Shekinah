#pragma once

#include "shekinah/application/cue_presenter.hpp"
#include "shekinah/ui/cue_view.hpp"
#include "shekinah/ui/program_status_types.hpp"

#include <QString>

#include <vector>

namespace shekinah::ui::main_window_internal {

struct CueViewSet {
    QString title;
    std::vector<CueView> cues;
    std::vector<QuickCueItem> quick_cues;
};

CueViewSet make_cue_views(const application::CuePresentationSet& presented);

}  // namespace shekinah::ui::main_window_internal
