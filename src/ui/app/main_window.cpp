#include "shekinah/ui/main_window.hpp"

#include "shekinah/application/resource_governor.hpp"
#include "shekinah/media/audience_output.hpp"
#include "shekinah/media/memory_diagnostics.hpp"
#include "shekinah/storage/content_repository.hpp"
#include "shekinah/ui/library_dock_widget.hpp"
#include "shekinah/ui/operator_page.hpp"
#include "shekinah/ui/program_status_widget.hpp"
#include "shekinah/ui/support/app_icon.hpp"

#include <QTimer>

#include <functional>
#include <utility>

namespace shekinah::ui {
namespace {

class WindowResourcePools final : public application::ResourcePools {
public:
    explicit WindowResourcePools(std::function<void()> purge_callback = {})
        : purge_callback_(std::move(purge_callback)) {}

    void set_purge_callback(std::function<void()> purge_callback) {
        purge_callback_ = std::move(purge_callback);
    }

    void purge_non_live_caches() override {
        if (purge_callback_) {
            purge_callback_();
        }
    }
    void cancel_queued_work() override {}
    void release_non_active_media() override {}

private:
    std::function<void()> purge_callback_;
};

}  // namespace

ShekinahWindow::ShekinahWindow(QWidget* parent) : QMainWindow(parent) {
    setObjectName(QStringLiteral("shekinah-window"));
    setWindowTitle(QStringLiteral("Shekinah"));
    setWindowIcon(app_icon());
    resize(1280, 720);

    resource_pools_ = std::make_unique<WindowResourcePools>();
    resource_governor_ = std::make_unique<application::ResourceGovernor>(*resource_pools_);
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
    static_cast<WindowResourcePools*>(resource_pools_.get())->set_purge_callback([this] {
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

media::AudienceOutput* ShekinahWindow::audience_output() {
    if (!audience_output_) {
        audience_output_ = std::make_unique<media::AudienceOutput>();
        audience_output_->setWindowIcon(app_icon());
        audience_output_->present(presentation_.program());
        present_live_media();
    }
    return audience_output_.get();
}

void ShekinahWindow::sample_memory_usage(std::size_t working_set_bytes) {
    if (resource_governor_) {
        resource_governor_->on_working_set_sample(working_set_bytes);
    }
    if (operator_page_ != nullptr && operator_page_->program_status() != nullptr) {
        operator_page_->program_status()->set_memory_status(
            last_working_set_bytes(), live_media_admitted());
    }
}

std::size_t ShekinahWindow::last_working_set_bytes() const noexcept {
    return resource_governor_ == nullptr ? 0 : resource_governor_->last_sample_bytes();
}

bool ShekinahWindow::live_media_admitted() const noexcept {
    return resource_governor_ == nullptr || resource_governor_->may_load_live_media();
}

domain::Status ShekinahWindow::open_project(const std::filesystem::path& root) {
    const auto opened = session_.open(root);
    if (!opened) {
        return opened;
    }

    next_content_id_ = 1;

    const auto order = session_.repository()->ensure_service_order(
        domain::ServiceOrderId{"default-service"}, "Service Order");
    if (!order) {
        session_.close();
        return order;
    }
    refresh_library();
    refresh_service_order();
    return domain::success();
}

}  // namespace shekinah::ui
