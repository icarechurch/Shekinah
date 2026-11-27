#include "window_resource_pools.hpp"

#include <utility>

namespace shekinah::ui {

WindowResourcePools::WindowResourcePools(std::function<void()> purge_callback)
    : purge_callback_(std::move(purge_callback)) {}

void WindowResourcePools::set_purge_callback(std::function<void()> purge_callback) {
    purge_callback_ = std::move(purge_callback);
}

void WindowResourcePools::purge_non_live_caches() {
    if (purge_callback_) {
        purge_callback_();
    }
}

void WindowResourcePools::cancel_queued_work() {}

void WindowResourcePools::release_non_active_media() {}

}  // namespace shekinah::ui
