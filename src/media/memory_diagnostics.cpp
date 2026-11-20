#include "shekinah/media/memory_diagnostics.hpp"

#include <fstream>
#include <string>

#if defined(_WIN32)
#include <windows.h>
#include <psapi.h>
#endif

namespace shekinah::media {

std::size_t MemoryDiagnostics::working_set_bytes() noexcept {
#if defined(_WIN32)
    PROCESS_MEMORY_COUNTERS counters{};
    counters.cb = sizeof(counters);
    if (GetProcessMemoryInfo(GetCurrentProcess(), &counters, sizeof(counters)) != 0) {
        return static_cast<std::size_t>(counters.WorkingSetSize);
    }
    return 0;
#else
    std::ifstream status{ "/proc/self/status" };
    std::string label;
    std::size_t kilobytes = 0;
    while (status >> label) {
        if (label == "VmRSS:") {
            status >> kilobytes;
            return kilobytes * 1024U;
        }
        status.ignore(1024, '\n');
    }
    return 0;
#endif
}

}  // namespace shekinah::media
