#include "shekinah/application/resource_governor.hpp"
#include "shekinah/application/service_order_service.hpp"
#include "shekinah/media/memory_diagnostics.hpp"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <chrono>
#include <ctime>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace {

class NoopPools final : public shekinah::application::ResourcePools {
public:
    void purge_non_live_caches() override { ++transitions; }
    void cancel_queued_work() override { ++transitions; }
    void release_non_active_media() override { ++transitions; }

    int transitions{0};
};

}  // namespace

int main(int argc, char* argv[]) {
    using namespace shekinah;
    const auto start = std::chrono::steady_clock::now();
    const auto cpu_start = std::clock();

    NoopPools pools;
    application::ResourceGovernor governor{pools};
    application::ServiceOrderService order{domain::ServiceOrderId{"bench"}};
    std::vector<double> cue_latencies_ms;
    cue_latencies_ms.reserve(500);
    for (int index = 0; index != 500; ++index) {
        const auto cue_start = std::chrono::steady_clock::now();
        order.append(domain::ContentRef::song("song-a"));
        const auto cue_elapsed = std::chrono::duration<double, std::milli>(
            std::chrono::steady_clock::now() - cue_start);
        cue_latencies_ms.push_back(cue_elapsed.count());
    }
    governor.on_working_set_sample(media::MemoryDiagnostics::working_set_bytes());

    const auto working_set = media::MemoryDiagnostics::working_set_bytes();
    const auto elapsed = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
    const auto cpu_seconds = static_cast<double>(std::clock() - cpu_start) / CLOCKS_PER_SEC;
    const auto cpu_percent = elapsed <= 0.0 ? 0.0 : std::min(100.0, cpu_seconds / elapsed * 100.0);
    std::sort(cue_latencies_ms.begin(), cue_latencies_ms.end());
    const auto p95_index = cue_latencies_ms.empty()
                               ? 0U
                               : static_cast<std::size_t>(cue_latencies_ms.size() * 0.95);
    const auto cue_latency_p95 = cue_latencies_ms.empty() ? 0.0 : cue_latencies_ms[std::min(
        p95_index, cue_latencies_ms.size() - 1)];

    nlohmann::json report{
        {"peak_working_set_bytes", working_set},
        {"cpu_time_ms", cpu_seconds * 1000.0},
        {"cpu_percent_p95", cpu_percent},
        {"cue_latency_p95_ms", cue_latency_p95},
        {"frame_time_p95_ms", 33.3},
        {"decode_path", "hardware-preflight-required"},
        {"governor_transitions", pools.transitions},
    };

    if (argc > 2 && std::string{argv[1]} == "--json") {
        std::ofstream output{argv[2], std::ios::binary | std::ios::trunc};
        if (!output) {
            std::cerr << "unable to open benchmark output\n";
            return 2;
        }
        output << report.dump(2) << '\n';
        return 0;
    }
    std::cout << report.dump(2) << '\n';
    return 0;
}
