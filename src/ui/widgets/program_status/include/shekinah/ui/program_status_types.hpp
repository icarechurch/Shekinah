#pragma once

#include "shekinah/domain/model.hpp"

#include <QString>

#include <vector>

namespace shekinah::ui {

struct LiveSlideDetails {
    QString item_title;
    QString section;
    QString slide_text;
    QString asset_name;
    QString next_section;
    QString next_summary;
    int slide_index{0};
    int total_slides{0};
    bool has_background{false};
    bool is_video{false};
    domain::FitMode fit_mode{domain::FitMode::cover};
};

struct QuickCueItem {
    domain::CueId id;
    int number{0};
    QString section;
    QString summary;
};

}  // namespace shekinah::ui
