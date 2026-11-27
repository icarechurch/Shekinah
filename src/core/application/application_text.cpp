#include "application_text.hpp"

#include <sstream>

namespace shekinah::application::internal {

std::string join_lines(const std::vector<std::string>& lines) {
    std::ostringstream out;
    for (std::size_t index = 0; index != lines.size(); ++index) {
        if (index != 0) {
            out << '\n';
        }
        out << lines[index];
    }
    return out.str();
}

}  // namespace shekinah::application::internal
