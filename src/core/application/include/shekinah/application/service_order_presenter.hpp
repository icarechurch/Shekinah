#pragma once

#include "shekinah/domain/model.hpp"

#include <string>

namespace shekinah::application {

struct ServiceOrderItemPresentation {
    std::string label;
    int slide_count{0};
    bool has_background{false};
    bool is_live{false};
};

[[nodiscard]] ServiceOrderItemPresentation present_service_order_item(
    const domain::ServiceOrderItem& item,
    const domain::Song* song,
    const domain::SermonDeck* sermon,
    const domain::ProgramState& program);

}  // namespace shekinah::application
