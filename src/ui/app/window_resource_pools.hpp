#pragma once

#include "shekinah/application/resource_governor.hpp"

#include <functional>

namespace shekinah::ui {

class WindowResourcePools final : public application::ResourcePools {
public:
    explicit WindowResourcePools(std::function<void()> purge_callback = {});

    void set_purge_callback(std::function<void()> purge_callback);
    void purge_non_live_caches() override;
    void cancel_queued_work() override;
    void release_non_active_media() override;

private:
    std::function<void()> purge_callback_;
};

}  // namespace shekinah::ui
