#pragma once

namespace shekinah::storage {
class ProjectSession;
}

namespace shekinah::ui {

class LibraryDockWidget;
class AssetCatalogCoordinator;

class LibraryCatalogCoordinator final {
public:
    LibraryCatalogCoordinator(storage::ProjectSession& session,
                              AssetCatalogCoordinator& assets) noexcept;

    void refresh(LibraryDockWidget& library);

private:
    storage::ProjectSession& session_;
    AssetCatalogCoordinator& assets_;
};

}  // namespace shekinah::ui
