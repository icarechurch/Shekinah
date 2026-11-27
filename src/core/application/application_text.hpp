#pragma once

#include <string>
#include <vector>

namespace shekinah::application::internal {

[[nodiscard]] std::string join_lines(const std::vector<std::string>& lines);

}  // namespace shekinah::application::internal
