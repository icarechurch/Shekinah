#include "shekinah/ui/main_window.hpp"

#include "shekinah/application/resource_governor.hpp"
#include "shekinah/media/audience_output.hpp"
#include "shekinah/media/memory_diagnostics.hpp"
#include "shekinah/ui/library_dock_widget.hpp"
#include "shekinah/ui/operator_page.hpp"
#include "shekinah/ui/support/app_icon.hpp"
#include "content_persistence_coordinator.hpp"
#include "library_catalog_coordinator.hpp"
#include "service_order_coordinator.hpp"
#include "window_resource_pools.hpp"

#include <QTimer>

namespace shekinah::ui {

ShekinahWindow::ShekinahWindow(QWidget* parent) : QMainWindow(parent) {
    setObjectName(QStringLiteral("shekinah-window"));
    setWindowTitle(QStringLiteral("Shekinah"));
    setWindowIcon(app_icon());
    resize(1280, 720);

    resource_pools_ = std::make_unique<WindowResourcePools>();
    resource_governor_ = std::make_unique<application::ResourceGovernor>(*resource_pools_);
    content_persistence_coordinator_ =
        std::make_unique<ContentPersistenceCoordinator>(session_, presentation_);
    library_catalog_coordinator_ = std::make_unique<LibraryCatalogCoordinator>(session_);
    service_order_coordinator_ = std::make_unique<ServiceOrderCoordinator>(session_);
    resource_timer_ = new QTimer{this};
    resource_timer_->setInterval(1000);
    QObject::connect(resource_timer_, &QTimer::timeout, this, [this] {
        sample_memory_usage(media::MemoryDiagnostics::working_set_bytes());
    });
    resource_timer_->start();

    operator_page_ = new OperatorPage{this};
    setCentralWidget(operator_page_);
    configure_operator();
    library_dock_ = new LibraryDockWidget{this};
    resource_pools_->set_purge_callback([this] {
        if (library_dock_ != nullptr) {
            library_dock_->purge_non_live_caches();
        }
    });
    addDockWidget(Qt::BottomDockWidgetArea, library_dock_);
    configure_feature_handlers();
    library_dock_->setMinimumHeight(120);
    library_dock_->show();
    resizeDocks({library_dock_}, {190}, Qt::Vertical);
    configure_window_actions();

    apply_theme();
    sync_theme_actions();
    QObject::connect(&ThemeManager::instance(), &ThemeManager::theme_changed, this,
                     [this](ThemeId) {
                         apply_theme();
                         sync_theme_actions();
                     });
}

ShekinahWindow::~ShekinahWindow() = default;

}  // namespace shekinah::ui
