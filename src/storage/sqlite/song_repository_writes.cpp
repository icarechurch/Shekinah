#include "shekinah/storage/content_repository.hpp"

#include "internal/sqlite_support.hpp"

#include <nlohmann/json.hpp>

#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace shekinah::storage {
namespace {

using sqlite_internal::Statement;
using sqlite_internal::bind_integer;
using sqlite_internal::bind_optional_text;
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

domain::Result<domain::Song> SqliteContentRepository::save_song(const domain::Song& song) {
    if (song.id.empty() || song.title.empty()) {
        return domain::Result<domain::Song>::fail(
            {domain::ErrorCode::invalid_input, "A Song needs a non-empty id and title"});
    }

    const auto status = store_.transaction([&](sqlite3* database) {
        Statement song_statement{
            database,
            "INSERT INTO songs(id,title,default_background_asset_id) VALUES(?1,?2,?3) "
            "ON CONFLICT(id) DO UPDATE SET title=excluded.title, "
            "default_background_asset_id=excluded.default_background_asset_id, archived=0;"};
        if (!song_statement.ready()) {
            return sqlite_failure(database, "Could not prepare Song save");
        }
        if (const auto bound = bind_text(song_statement, 1, song.id.value); !bound) {
            return bound;
        }
        if (const auto bound = bind_text(song_statement, 2, song.title); !bound) {
            return bound;
        }
        const std::optional<std::string> background = song.default_background.has_value()
                                                          ? std::optional<std::string>{
                                                                song.default_background->asset.value}
                                                          : std::nullopt;
        if (const auto bound = bind_optional_text(song_statement, 3, background); !bound) {
            return bound;
        }
        if (const auto stepped = step_done(song_statement, "Song save failed"); !stepped) {
            return stepped;
        }

        for (const char* query : {"DELETE FROM song_slides WHERE song_id=?1;",
                                  "DELETE FROM song_slide_breaks WHERE song_id=?1;",
                                  "DELETE FROM song_reflow_documents WHERE song_id=?1;"}) {
            Statement deletion{database, query};
            if (!deletion.ready()) {
                return sqlite_failure(database, "Could not prepare Song cleanup");
            }
            if (const auto bound = bind_text(deletion, 1, song.id.value); !bound) {
                return bound;
            }
            if (const auto stepped = step_done(deletion, "Song cleanup failed"); !stepped) {
                return stepped;
            }
        }

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
            if (const auto bound = bind_integer(break_insert, 2, static_cast<std::int64_t>(index)); !bound) {
                return bound;
            }
            if (const auto bound = bind_integer(
                    break_insert, 3, static_cast<std::int64_t>(break_offsets[index]));
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
            if (const auto bound = bind_integer(slide_insert, 3, static_cast<std::int64_t>(index)); !bound) {
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
    });

    if (!status) {
        return domain::Result<domain::Song>::fail(status.error());
    }
    return domain::Result<domain::Song>::ok(song);
}

}  // namespace shekinah::storage
