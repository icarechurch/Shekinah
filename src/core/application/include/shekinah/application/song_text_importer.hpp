#pragma once

#include "shekinah/domain/model.hpp"
#include "shekinah/domain/result.hpp"

#include <string_view>

namespace shekinah::application {

class SongTextImporter {
public:
    static domain::Result<domain::Song> parse(std::string_view text);
};

}  // namespace shekinah::application
