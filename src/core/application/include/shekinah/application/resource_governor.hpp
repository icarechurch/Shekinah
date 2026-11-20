#pragma once

#include <cstddef>

namespace shekinah::application {

class ResourcePools {
public:
    virtual ~ResourcePools() = default;
    virtual void purge_non_live_caches() = 0;
    virtual void cancel_queued_work() = 0;
    virtual void release_non_active_media() = 0;
};

class ResourceGovernor {
public:
    static constexpr std::size_t purge_threshold_bytes = 220'000'000;
    static constexpr std::size_t release_threshold_bytes = 250'000'000;
    static constexpr std::size_t reject_threshold_bytes = 270'000'000;
    static constexpr std::size_t hard_limit_bytes = 300'000'000;

    explicit ResourceGovernor(ResourcePools& pools) : pools_(pools) {}

    void on_working_set_sample(std::size_t bytes);
    [[nodiscard]] bool may_load_live_media() const noexcept { return may_load_live_media_; }
    [[nodiscard]] std::size_t last_sample_bytes() const noexcept { return last_sample_bytes_; }

private:
    ResourcePools& pools_;
    std::size_t last_sample_bytes_{0};
    int pressure_level_{0};
    bool may_load_live_media_{true};
};

}  // namespace shekinah::application
