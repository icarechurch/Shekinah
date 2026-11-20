#include "shekinah/application/resource_governor.hpp"

namespace shekinah::application {

void ResourceGovernor::on_working_set_sample(std::size_t bytes) {
    last_sample_bytes_ = bytes;
    if (bytes >= reject_threshold_bytes) {
        if (pressure_level_ < 1) {
            pools_.purge_non_live_caches();
        }
        if (pressure_level_ < 2) {
            pools_.cancel_queued_work();
            pools_.release_non_active_media();
        }
        pressure_level_ = 3;
        may_load_live_media_ = false;
        return;
    }
    if (bytes >= release_threshold_bytes) {
        if (pressure_level_ < 1) {
            pools_.purge_non_live_caches();
        }
        if (pressure_level_ < 2) {
            pools_.cancel_queued_work();
            pools_.release_non_active_media();
        }
        pressure_level_ = 2;
        may_load_live_media_ = true;
        return;
    }
    if (bytes >= purge_threshold_bytes) {
        if (pressure_level_ < 1) {
            pools_.purge_non_live_caches();
        }
        pressure_level_ = 1;
        may_load_live_media_ = true;
        return;
    }
    pressure_level_ = 0;
    may_load_live_media_ = true;
}

}  // namespace shekinah::application
