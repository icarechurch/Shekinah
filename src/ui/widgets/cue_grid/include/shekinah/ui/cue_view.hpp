#pragma once

#include "shekinah/domain/model.hpp"

#include <QString>

#include <utility>

namespace shekinah::ui {

struct CueView {
    domain::CueId id;
    QString section;
    QString summary;
    int number{0};
    QString background_path;
    domain::FitMode background_fit{domain::FitMode::cover};
    QString footer_label;
    bool show_text{true};

    CueView(domain::CueId cue_id,
            QString cue_section,
            QString cue_summary,
            int cue_number = 0,
            QString cue_background_path = {},
            domain::FitMode cue_background_fit = domain::FitMode::cover,
            QString cue_footer_label = {},
            bool cue_show_text = true)
        : id(std::move(cue_id)),
          section(std::move(cue_section)),
          summary(std::move(cue_summary)),
          number(cue_number),
          background_path(std::move(cue_background_path)),
          background_fit(cue_background_fit),
          footer_label(std::move(cue_footer_label)),
          show_text(cue_show_text) {}
};

}  // namespace shekinah::ui
