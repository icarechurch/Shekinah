#pragma once

namespace shekinah::storage {
class ProjectSession;
}

namespace shekinah::ui {

class LibraryDockWidget;

class LibraryCatalogCoordinator final {
public:
    explicit LibraryCatalogCoordinator(storage::ProjectSession& session) noexcept;

    void refresh(LibraryDockWidget& library);

private:
    storage::ProjectSession& session_;
};

}  // namespace shekinah::ui
