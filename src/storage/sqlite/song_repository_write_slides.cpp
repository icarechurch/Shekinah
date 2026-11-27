#include "internal/song_repository_writes.hpp"

#include "internal/sqlite_support.hpp"

#include <nlohmann/json.hpp>

#include <optional>

namespace shekinah::storage::song_repository_internal {
namespace {

using sqlite_internal::Statement;
using sqlite_internal::bind_integer;
using sqlite_internal::bind_optional_text;
using sqlite_internal::bind_text;
using sqlite_internal::sqlite_failure;
using sqlite_internal::step_done;

}  // namespace

domain::Status write_slides(sqlite3* database, const domain::Song& song) {
    Statement slide_insert{
        database,
        "INSERT INTO song_slides(id,song_id,ordinal,section_label,lines_json,background_asset_id) "
        "VALUES(?1,?2,?3,?4,?5,?6);"};
    if (!slide_insert.ready()) {
        return sqlite_failure(database, "Could not prepare Song slide save");
    }
    for (std::size_t index = 0; index != song.slides.size(); ++index) {
        const auto& slide = song.slides[index];
        sqlite3_reset(slide_insert.get());
        sqlite3_clear_bindings(slide_insert.get());
        if (const auto bound = bind_text(slide_insert, 1, slide.id.value); !bound) {
            return bound;
        }
        if (const auto bound = bind_text(slide_insert, 2, song.id.value); !bound) {
            return bound;
        }
        if (const auto bound = bind_integer(slide_insert, 3, static_cast<std::int64_t>(index));
            !bound) {
            return bound;
        }
        if (const auto bound = bind_text(slide_insert, 4, slide.section); !bound) {
            return bound;
        }
        const auto lines_json = nlohmann::json(slide.lines).dump();
        if (const auto bound = bind_text(slide_insert, 5, lines_json); !bound) {
            return bound;
        }
        const std::optional<std::string> background = slide.background.has_value()
                                                          ? std::optional<std::string>{
                                                                slide.background->asset.value}
                                                          : std::nullopt;
        if (const auto bound = bind_optional_text(slide_insert, 6, background); !bound) {
            return bound;
        }
        if (const auto stepped = step_done(slide_insert, "Song slide save failed"); !stepped) {
            return stepped;
        }
    }
    return domain::success();
}

}  // namespace shekinah::storage::song_repository_internal
