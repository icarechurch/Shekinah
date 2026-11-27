#pragma once

#include "shekinah/domain/result.hpp"

#include <filesystem>

namespace shekinah::application {
class PresentationService;
}

namespace shekinah::storage {
class ProjectSession;
}

namespace shekinah::ui {
class ServiceOrderCoordinator;

class ProjectLifecycleCoordinator final {
public:
    ProjectLifecycleCoordinator(storage::ProjectSession& session,
                                application::PresentationService& presentation,
                                ServiceOrderCoordinator& service_orders) noexcept;

    [[nodiscard]] domain::Status open(const std::filesystem::path& root);
    void close() noexcept;

private:
    storage::ProjectSession& session_;
    application::PresentationService& presentation_;
    ServiceOrderCoordinator& service_orders_;
};

}  // namespace shekinah::ui
