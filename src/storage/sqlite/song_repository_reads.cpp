#include "shekinah/storage/content_repository.hpp"

#include "internal/sqlite_support.hpp"

#include <nlohmann/json.hpp>

#include <string>
#include <utility>
#include <vector>

namespace shekinah::storage {
namespace {

using sqlite_internal::Statement;
using sqlite_internal::bind_text;

}  // namespace

domain::Result<domain::Song> SqliteContentRepository::load_song(const domain::SongId& id) const {
    Statement song_statement{store_.native_handle(),
                             "SELECT title,default_background_asset_id FROM songs "
                             "WHERE id=?1 AND archived=0 LIMIT 1;"};
    if (!song_statement.ready()) {
        return domain::Result<domain::Song>::fail(
            {domain::ErrorCode::storage_failure, "Could not prepare Song load"});
    }
    if (const auto bound = bind_text(song_statement, 1, id.value); !bound) {
        return domain::Result<domain::Song>::fail(bound.error());
    }
    if (sqlite3_step(song_statement.get()) != SQLITE_ROW) {
        return domain::Result<domain::Song>::fail(
            {domain::ErrorCode::not_found, "Song was not found"});
    }
    const auto* raw_title = sqlite3_column_text(song_statement.get(), 0);
    if (raw_title == nullptr) {
        return domain::Result<domain::Song>::fail(
            {domain::ErrorCode::storage_failure, "Song title is NULL"});
    }
    std::optional<domain::BackgroundRef> default_background;
    const auto* raw_background = sqlite3_column_text(song_statement.get(), 1);
    if (raw_background != nullptr) {
        default_background = domain::BackgroundRef{
            domain::AssetId{reinterpret_cast<const char*>(raw_background)}, domain::FitMode::cover};
    }

    Statement slide_statement{store_.native_handle(),
                              "SELECT id,section_label,lines_json,background_asset_id FROM song_slides "
                              "WHERE song_id=?1 ORDER BY ordinal ASC;"};
    if (!slide_statement.ready()) {
        return domain::Result<domain::Song>::fail(
            {domain::ErrorCode::storage_failure, "Could not prepare Song slide load"});
    }
    if (const auto bound = bind_text(slide_statement, 1, id.value); !bound) {
        return domain::Result<domain::Song>::fail(bound.error());
    }

    std::vector<domain::Slide> slides;
    int result = SQLITE_ROW;
    while ((result = sqlite3_step(slide_statement.get())) == SQLITE_ROW) {
        const auto* raw_id = sqlite3_column_text(slide_statement.get(), 0);
        const auto* raw_section = sqlite3_column_text(slide_statement.get(), 1);
        const auto* raw_lines = sqlite3_column_text(slide_statement.get(), 2);
        if (raw_id == nullptr || raw_section == nullptr || raw_lines == nullptr) {
            return domain::Result<domain::Song>::fail(
                {domain::ErrorCode::storage_failure, "Song slide contains NULL text"});
        }
        std::vector<std::string> lines;
        try {
            lines = nlohmann::json::parse(reinterpret_cast<const char*>(raw_lines))
                        .get<std::vector<std::string>>();
        } catch (const nlohmann::json::exception&) {
            return domain::Result<domain::Song>::fail(
                {domain::ErrorCode::storage_failure, "Song slide lines contain invalid JSON"});
        }
        std::optional<domain::BackgroundRef> background;
        const auto* raw_slide_background = sqlite3_column_text(slide_statement.get(), 3);
        if (raw_slide_background != nullptr) {
            background = domain::BackgroundRef{
                domain::AssetId{reinterpret_cast<const char*>(raw_slide_background)},
                domain::FitMode::cover,
            };
        }
        slides.push_back(domain::Slide{
            domain::CueId{reinterpret_cast<const char*>(raw_id)},
            reinterpret_cast<const char*>(raw_section),
            std::move(lines),
            background,
        });
    }
    if (result != SQLITE_DONE) {
        return domain::Result<domain::Song>::fail(
            {domain::ErrorCode::storage_failure, "Could not iterate Song slides"});
    }
    return domain::Result<domain::Song>::ok(domain::Song{
        id,
        reinterpret_cast<const char*>(raw_title),
        default_background,
        std::move(slides),
    });
}

domain::Result<std::vector<domain::Song>> SqliteContentRepository::list_songs() const {
    Statement statement{store_.native_handle(),
                        "SELECT id FROM songs WHERE archived=0 ORDER BY title COLLATE NOCASE ASC, id ASC;"};
    if (!statement.ready()) {
        return domain::Result<std::vector<domain::Song>>::fail(
            {domain::ErrorCode::storage_failure, "Could not prepare Song listing"});
    }

    std::vector<domain::Song> songs;
    int result = SQLITE_ROW;
    while ((result = sqlite3_step(statement.get())) == SQLITE_ROW) {
        const auto* raw_id = sqlite3_column_text(statement.get(), 0);
        if (raw_id == nullptr) {
            return domain::Result<std::vector<domain::Song>>::fail(
                {domain::ErrorCode::storage_failure, "Song listing contains a NULL id"});
        }
        const auto song = load_song(domain::SongId{reinterpret_cast<const char*>(raw_id)});
        if (!song) {
            return domain::Result<std::vector<domain::Song>>::fail(song.error());
        }
        songs.push_back(*song);
    }
    if (result != SQLITE_DONE) {
        return domain::Result<std::vector<domain::Song>>::fail(
            {domain::ErrorCode::storage_failure, "Could not iterate Song listing"});
    }
    return domain::Result<std::vector<domain::Song>>::ok(std::move(songs));
}

}  // namespace shekinah::storage
