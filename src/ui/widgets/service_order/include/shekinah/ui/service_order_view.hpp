#pragma once

#include "shekinah/domain/model.hpp"

#include <QString>

namespace shekinah::ui {

struct ServiceOrderView {
    domain::ServiceOrderItem item;
    QString label;
    int slide_count{0};
    bool has_background{false};
    bool is_live{false};
};

}  // namespace shekinah::ui
