#pragma once

#include "shekinah/application/presentation_service.hpp"
#include "shekinah/domain/model.hpp"
#include "shekinah/storage/content_repository.hpp"
#include "shekinah/ui/service_order_view.hpp"

#include <vector>

namespace shekinah::ui::main_window_internal {

[[nodiscard]] std::vector<ServiceOrderView> make_service_order_views(
    const storage::SqliteContentRepository& repository,
    application::PresentationService& presentation,
    const std::vector<domain::ServiceOrderItem>& items,
    const domain::ProgramState& program);

}  // namespace shekinah::ui::main_window_internal
