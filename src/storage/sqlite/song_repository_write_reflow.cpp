#include "internal/song_repository_writes.hpp"

#include "internal/sqlite_support.hpp"

#include <optional>
#include <string>
#include <vector>

namespace shekinah::storage::song_repository_internal {
namespace {

using sqlite_internal::Statement;
using sqlite_internal::bind_integer;
using sqlite_internal::bind_text;
using sqlite_internal::sqlite_failure;
using sqlite_internal::step_done;

std::string slide_text(const domain::Song& song, std::vector<std::size_t>& break_offsets) {
    std::string text;
    for (std::size_t index = 0; index != song.slides.size(); ++index) {
        if (index != 0) {
            break_offsets.push_back(text.size());
            text.push_back('\n');
        }
        for (std::size_t line = 0; line != song.slides[index].lines.size(); ++line) {
            if (line != 0) {
                text.push_back('\n');
            }
            text += song.slides[index].lines[line];
        }
    }
    return text;
}

}  // namespace

domain::Status write_reflow(sqlite3* database, const domain::Song& song) {
    std::vector<std::size_t> break_offsets;
    const auto reflow_text = slide_text(song, break_offsets);
    Statement reflow{database,
                     "INSERT INTO song_reflow_documents(song_id,utf8_text) VALUES(?1,?2);"};
    if (!reflow.ready()) {
        return sqlite_failure(database, "Could not prepare Reflow save");
    }
    if (const auto bound = bind_text(reflow, 1, song.id.value); !bound) {
        return bound;
    }
    if (const auto bound = bind_text(reflow, 2, reflow_text); !bound) {
        return bound;
    }
    if (const auto stepped = step_done(reflow, "Reflow save failed"); !stepped) {
        return stepped;
    }

    Statement break_insert{database,
                           "INSERT INTO song_slide_breaks(song_id,ordinal,utf8_offset,section_label) "
                           "VALUES(?1,?2,?3,?4);"};
    if (!break_insert.ready()) {
        return sqlite_failure(database, "Could not prepare Reflow break save");
    }
    for (std::size_t index = 0; index != break_offsets.size(); ++index) {
        sqlite3_reset(break_insert.get());
        sqlite3_clear_bindings(break_insert.get());
        if (const auto bound = bind_text(break_insert, 1, song.id.value); !bound) {
            return bound;
        }
        if (const auto bound = bind_integer(break_insert, 2, static_cast<std::int64_t>(index));
            !bound) {
            return bound;
        }
        if (const auto bound =
                bind_integer(break_insert, 3, static_cast<std::int64_t>(break_offsets[index]));
            !bound) {
            return bound;
        }
        if (const auto bound = bind_text(break_insert, 4, song.slides[index + 1].section); !bound) {
            return bound;
        }
        if (const auto stepped = step_done(break_insert, "Reflow break save failed"); !stepped) {
            return stepped;
        }
    }
    return domain::success();
}

}  // namespace shekinah::storage::song_repository_internal
