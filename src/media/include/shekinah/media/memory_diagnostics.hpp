#pragma once

#include <cstddef>

namespace shekinah::media {

class MemoryDiagnostics {
public:
    [[nodiscard]] static std::size_t working_set_bytes() noexcept;
};

}  // namespace shekinah::media
