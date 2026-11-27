#include "main_window_cue_views.hpp"

namespace shekinah::ui::main_window_internal {

CueViewSet make_cue_views(const application::CuePresentationSet& presented) {
    CueViewSet views;
    views.title = QString::fromStdString(presented.title);
    views.cues.reserve(presented.cues.size());
    for (const auto& cue : presented.cues) {
        views.cues.push_back(CueView{
            cue.id,
            QString::fromStdString(cue.section),
            QString::fromStdString(cue.summary),
            cue.number,
            QString::fromStdString(cue.background_path.string()),
            cue.background_fit,
            QString::fromStdString(cue.footer_label),
            cue.show_text,
        });
    }

    views.quick_cues.reserve(presented.quick_cues.size());
    for (const auto& cue : presented.quick_cues) {
        views.quick_cues.push_back(QuickCueItem{
            cue.id,
            cue.number,
            QString::fromStdString(cue.label),
            QString::fromStdString(cue.summary),
        });
    }
    return views;
}

}  // namespace shekinah::ui::main_window_internal
