#include "internal/sqlite_song_repository.hpp"

#include "internal/sqlite_support.hpp"

#include <string>
#include <utility>
#include <vector>

namespace shekinah::storage {
namespace {

using sqlite_internal::Statement;
using sqlite_internal::bind_text;

}  // namespace

domain::Result<application::SongReflowDocument> SqliteSongRepository::load_song_reflow(
    const domain::SongId& id) const {
    Statement document{store_.native_handle(),
                       "SELECT utf8_text FROM song_reflow_documents WHERE song_id=?1 LIMIT 1;"};
    if (!document.ready()) {
        return domain::Result<application::SongReflowDocument>::fail(
            {domain::ErrorCode::storage_failure, "Could not prepare Reflow document load"});
    }
    if (const auto bound = bind_text(document, 1, id.value); !bound) {
        return domain::Result<application::SongReflowDocument>::fail(bound.error());
    }
    if (sqlite3_step(document.get()) != SQLITE_ROW) {
        return domain::Result<application::SongReflowDocument>::fail(
            {domain::ErrorCode::not_found, "Reflow document was not found"});
    }
    const auto* raw_text = sqlite3_column_text(document.get(), 0);
    if (raw_text == nullptr) {
        return domain::Result<application::SongReflowDocument>::fail(
            {domain::ErrorCode::storage_failure, "Reflow document text is NULL"});
    }

    Statement breaks{store_.native_handle(),
                     "SELECT utf8_offset,section_label FROM song_slide_breaks "
                     "WHERE song_id=?1 ORDER BY ordinal ASC;"};
    if (!breaks.ready()) {
        return domain::Result<application::SongReflowDocument>::fail(
            {domain::ErrorCode::storage_failure, "Could not prepare Reflow break load"});
    }
    if (const auto bound = bind_text(breaks, 1, id.value); !bound) {
        return domain::Result<application::SongReflowDocument>::fail(bound.error());
    }
    std::vector<application::SlideBreak> slide_breaks;
    int result = SQLITE_ROW;
    while ((result = sqlite3_step(breaks.get())) == SQLITE_ROW) {
        const auto* raw_label = sqlite3_column_text(breaks.get(), 1);
        if (raw_label == nullptr) {
            return domain::Result<application::SongReflowDocument>::fail(
                {domain::ErrorCode::storage_failure, "Reflow break label is NULL"});
        }
        slide_breaks.push_back(application::SlideBreak{
            static_cast<std::size_t>(sqlite3_column_int64(breaks.get(), 0)),
            reinterpret_cast<const char*>(raw_label),
        });
    }
    if (result != SQLITE_DONE) {
        return domain::Result<application::SongReflowDocument>::fail(
            {domain::ErrorCode::storage_failure, "Could not iterate Reflow breaks"});
    }
    return domain::Result<application::SongReflowDocument>::ok(
        application::SongReflowDocument{reinterpret_cast<const char*>(raw_text), std::move(slide_breaks)});
}

}  // namespace shekinah::storage
