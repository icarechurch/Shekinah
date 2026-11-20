#include "shekinah/storage/content_repository.hpp"

#include <nlohmann/json.hpp>

#include <optional>
#include <string>
#include <string_view>
#include <utility>

namespace shekinah::storage {
namespace {

class Statement {
public:
    Statement(sqlite3* database, const char* sql) : database_(database) {
        result_ = sqlite3_prepare_v2(database_, sql, -1, &statement_, nullptr);
    }

    Statement(const Statement&) = delete;
    Statement& operator=(const Statement&) = delete;

    ~Statement() {
        if (statement_ != nullptr) {
            sqlite3_finalize(statement_);
        }
    }

    [[nodiscard]] bool ready() const noexcept { return result_ == SQLITE_OK; }
    [[nodiscard]] sqlite3_stmt* get() const noexcept { return statement_; }
    [[nodiscard]] sqlite3* database() const noexcept { return database_; }

private:
    sqlite3* database_;
    sqlite3_stmt* statement_{nullptr};
    int result_{SQLITE_ERROR};
};

domain::Status sqlite_failure(sqlite3* database, std::string context) {
    context += ": ";
    context += database == nullptr ? "SQLite error" : sqlite3_errmsg(database);
    return domain::Status::fail({domain::ErrorCode::storage_failure, std::move(context)});
}

domain::Status bind_text(Statement& statement, int index, const std::string& value) {
    if (sqlite3_bind_text(statement.get(), index, value.c_str(), static_cast<int>(value.size()),
                          SQLITE_TRANSIENT) != SQLITE_OK) {
        return sqlite_failure(statement.database(), "SQLite text bind failed");
    }
    return domain::success();
}

domain::Status bind_optional_text(Statement& statement, int index, const std::optional<std::string>& value) {
    const int result = value.has_value()
                           ? sqlite3_bind_text(statement.get(), index, value->c_str(),
                                               static_cast<int>(value->size()), SQLITE_TRANSIENT)
                           : sqlite3_bind_null(statement.get(), index);
    if (result != SQLITE_OK) {
        return sqlite_failure(statement.database(), "SQLite optional text bind failed");
    }
    return domain::success();
}

domain::Status bind_integer(Statement& statement, int index, std::int64_t value) {
    if (sqlite3_bind_int64(statement.get(), index, value) != SQLITE_OK) {
        return sqlite_failure(statement.database(), "SQLite integer bind failed");
    }
    return domain::success();
}

domain::Status step_done(Statement& statement, const char* operation) {
    if (sqlite3_step(statement.get()) != SQLITE_DONE) {
        return sqlite_failure(statement.database(), operation);
    }
    return domain::success();
}

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

domain::Result<bool> content_exists(sqlite3* database, const domain::ContentRef& content) {
    const char* query = content.kind == domain::ContentKind::song
                             ? "SELECT 1 FROM songs WHERE id=?1 AND archived=0 LIMIT 1;"
                             : "SELECT 1 FROM sermon_decks WHERE id=?1 AND archived=0 LIMIT 1;";
    Statement statement{database, query};
    if (!statement.ready()) {
        return domain::Result<bool>::fail(
            {domain::ErrorCode::storage_failure, "Could not prepare content lookup"});
    }
    const auto id = content.kind == domain::ContentKind::song
                        ? std::get<domain::SongId>(content.id).value
                        : std::get<domain::SermonId>(content.id).value;
    if (const auto bound = bind_text(statement, 1, id); !bound) {
        return domain::Result<bool>::fail(bound.error());
    }
    return domain::Result<bool>::ok(sqlite3_step(statement.get()) == SQLITE_ROW);
}

domain::Status archive_content(ProjectStore& store, std::string table, std::string kind, std::string id) {
    if (id.empty()) {
        return domain::Status::fail({domain::ErrorCode::invalid_input, "Reusable content needs an id"});
    }
    return store.transaction([&](sqlite3* database) {
        Statement references{database,
                             "SELECT COUNT(*) FROM service_order_items "
                             "WHERE content_kind=?1 AND content_id=?2;"};
        if (!references.ready()) {
            return sqlite_failure(database, "Could not prepare reference count");
        }
        if (const auto bound = bind_text(references, 1, kind); !bound) {
            return bound;
        }
        if (const auto bound = bind_text(references, 2, id); !bound) {
            return bound;
        }
        if (sqlite3_step(references.get()) != SQLITE_ROW) {
            return sqlite_failure(database, "Could not read reference count");
        }
        if (sqlite3_column_int64(references.get(), 0) != 0) {
            return domain::Status::fail(
                {domain::ErrorCode::invalid_input, "Remove service-order references before archiving content"});
        }

        const auto query = "UPDATE " + table + " SET archived=1 WHERE id=?1 AND archived=0;";
        Statement update{database, query.c_str()};
        if (!update.ready()) {
            return sqlite_failure(database, "Could not prepare archive operation");
        }
        if (const auto bound = bind_text(update, 1, id); !bound) {
            return bound;
        }
        if (const auto stepped = step_done(update, "Archive operation failed"); !stepped) {
            return stepped;
        }
        if (sqlite3_changes(database) == 0) {
            return domain::Status::fail({domain::ErrorCode::not_found, "Reusable content was not found"});
        }
        return domain::success();
    });
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

domain::Result<domain::SermonDeck> SqliteContentRepository::save_deck(const domain::SermonDeck& deck) {
    if (deck.id.empty() || deck.title.empty()) {
        return domain::Result<domain::SermonDeck>::fail(
            {domain::ErrorCode::invalid_input, "A Sermon deck needs a non-empty id and title"});
    }

    const auto status = store_.transaction([&](sqlite3* database) {
        Statement deck_insert{database,
                              "INSERT INTO sermon_decks(id,title) VALUES(?1,?2) "
                              "ON CONFLICT(id) DO UPDATE SET title=excluded.title, archived=0;"};
        if (!deck_insert.ready()) {
            return sqlite_failure(database, "Could not prepare Sermon save");
        }
        if (const auto bound = bind_text(deck_insert, 1, deck.id.value); !bound) {
            return bound;
        }
        if (const auto bound = bind_text(deck_insert, 2, deck.title); !bound) {
            return bound;
        }
        if (const auto stepped = step_done(deck_insert, "Sermon save failed"); !stepped) {
            return stepped;
        }

        Statement delete_slides{database, "DELETE FROM sermon_slides WHERE sermon_id=?1;"};
        if (!delete_slides.ready()) {
            return sqlite_failure(database, "Could not prepare Sermon cleanup");
        }
        if (const auto bound = bind_text(delete_slides, 1, deck.id.value); !bound) {
            return bound;
        }
        if (const auto stepped = step_done(delete_slides, "Sermon cleanup failed"); !stepped) {
            return stepped;
        }

        Statement slide_insert{database,
                               "INSERT INTO sermon_slides(id,sermon_id,ordinal,image_asset_id) "
                               "VALUES(?1,?2,?3,?4);"};
        if (!slide_insert.ready()) {
            return sqlite_failure(database, "Could not prepare Sermon slide save");
        }
        for (std::size_t index = 0; index != deck.slides.size(); ++index) {
            const auto& slide = deck.slides[index];
            sqlite3_reset(slide_insert.get());
            sqlite3_clear_bindings(slide_insert.get());
            if (const auto bound = bind_text(slide_insert, 1, slide.id.value); !bound) {
                return bound;
            }
            if (const auto bound = bind_text(slide_insert, 2, deck.id.value); !bound) {
                return bound;
            }
            if (const auto bound = bind_integer(slide_insert, 3, static_cast<std::int64_t>(index)); !bound) {
                return bound;
            }
            if (const auto bound = bind_text(slide_insert, 4, slide.image.value); !bound) {
                return bound;
            }
            if (const auto stepped = step_done(slide_insert, "Sermon slide save failed"); !stepped) {
                return stepped;
            }
        }
        return domain::success();
    });

    if (!status) {
        return domain::Result<domain::SermonDeck>::fail(status.error());
    }
    return domain::Result<domain::SermonDeck>::ok(deck);
}

domain::Result<domain::SermonDeck> SqliteContentRepository::load_deck(const domain::SermonId& id) const {
    Statement deck_statement{store_.native_handle(),
                             "SELECT title FROM sermon_decks WHERE id=?1 AND archived=0 LIMIT 1;"};
    if (!deck_statement.ready()) {
        return domain::Result<domain::SermonDeck>::fail(
            {domain::ErrorCode::storage_failure, "Could not prepare Sermon load"});
    }
    if (const auto bound = bind_text(deck_statement, 1, id.value); !bound) {
        return domain::Result<domain::SermonDeck>::fail(bound.error());
    }
    if (sqlite3_step(deck_statement.get()) != SQLITE_ROW) {
        return domain::Result<domain::SermonDeck>::fail(
            {domain::ErrorCode::not_found, "Sermon deck was not found"});
    }
    const auto* raw_title = sqlite3_column_text(deck_statement.get(), 0);
    if (raw_title == nullptr) {
        return domain::Result<domain::SermonDeck>::fail(
            {domain::ErrorCode::storage_failure, "Sermon title is NULL"});
    }

    Statement slide_statement{store_.native_handle(),
                              "SELECT id,image_asset_id FROM sermon_slides "
                              "WHERE sermon_id=?1 ORDER BY ordinal ASC;"};
    if (!slide_statement.ready()) {
        return domain::Result<domain::SermonDeck>::fail(
            {domain::ErrorCode::storage_failure, "Could not prepare Sermon slide load"});
    }
    if (const auto bound = bind_text(slide_statement, 1, id.value); !bound) {
        return domain::Result<domain::SermonDeck>::fail(bound.error());
    }

    std::vector<domain::SermonSlide> slides;
    int result = SQLITE_ROW;
    while ((result = sqlite3_step(slide_statement.get())) == SQLITE_ROW) {
        const auto* raw_slide_id = sqlite3_column_text(slide_statement.get(), 0);
        const auto* raw_asset_id = sqlite3_column_text(slide_statement.get(), 1);
        if (raw_slide_id == nullptr || raw_asset_id == nullptr) {
            return domain::Result<domain::SermonDeck>::fail(
                {domain::ErrorCode::storage_failure, "Sermon slide contains NULL text"});
        }
        slides.push_back(domain::SermonSlide{
            domain::CueId{reinterpret_cast<const char*>(raw_slide_id)},
            domain::AssetId{reinterpret_cast<const char*>(raw_asset_id)},
        });
    }
    if (result != SQLITE_DONE) {
        return domain::Result<domain::SermonDeck>::fail(
            {domain::ErrorCode::storage_failure, "Could not iterate Sermon slides"});
    }
    return domain::Result<domain::SermonDeck>::ok(domain::SermonDeck{
        id,
        reinterpret_cast<const char*>(raw_title),
        std::move(slides),
    });
}

domain::Result<std::vector<domain::SermonDeck>> SqliteContentRepository::list_decks() const {
    Statement statement{store_.native_handle(),
                        "SELECT id FROM sermon_decks WHERE archived=0 "
                        "ORDER BY title COLLATE NOCASE ASC, id ASC;"};
    if (!statement.ready()) {
        return domain::Result<std::vector<domain::SermonDeck>>::fail(
            {domain::ErrorCode::storage_failure, "Could not prepare Sermon listing"});
    }

    std::vector<domain::SermonDeck> decks;
    int result = SQLITE_ROW;
    while ((result = sqlite3_step(statement.get())) == SQLITE_ROW) {
        const auto* raw_id = sqlite3_column_text(statement.get(), 0);
        if (raw_id == nullptr) {
            return domain::Result<std::vector<domain::SermonDeck>>::fail(
                {domain::ErrorCode::storage_failure, "Sermon listing contains a NULL id"});
        }
        const auto deck = load_deck(domain::SermonId{reinterpret_cast<const char*>(raw_id)});
        if (!deck) {
            return domain::Result<std::vector<domain::SermonDeck>>::fail(deck.error());
        }
        decks.push_back(*deck);
    }
    if (result != SQLITE_DONE) {
        return domain::Result<std::vector<domain::SermonDeck>>::fail(
            {domain::ErrorCode::storage_failure, "Could not iterate Sermon listing"});
    }
    return domain::Result<std::vector<domain::SermonDeck>>::ok(std::move(decks));
}

domain::Result<std::vector<domain::AssetRecord>> SqliteContentRepository::list_assets() const {
    Statement statement{store_.native_handle(),
                        "SELECT id,kind,sha256,relative_path,size_bytes FROM assets WHERE archived=0 "
                        "ORDER BY relative_path COLLATE NOCASE ASC, id ASC;"};
    if (!statement.ready()) {
        return domain::Result<std::vector<domain::AssetRecord>>::fail(
            {domain::ErrorCode::storage_failure, "Could not prepare Asset listing"});
    }

    std::vector<domain::AssetRecord> assets;
    int result = SQLITE_ROW;
    while ((result = sqlite3_step(statement.get())) == SQLITE_ROW) {
        const auto* raw_id = sqlite3_column_text(statement.get(), 0);
        const auto* raw_kind = sqlite3_column_text(statement.get(), 1);
        const auto* raw_hash = sqlite3_column_text(statement.get(), 2);
        const auto* raw_path = sqlite3_column_text(statement.get(), 3);
        if (raw_id == nullptr || raw_kind == nullptr || raw_hash == nullptr || raw_path == nullptr) {
            return domain::Result<std::vector<domain::AssetRecord>>::fail(
                {domain::ErrorCode::storage_failure, "Asset listing contains NULL metadata"});
        }
        assets.push_back(domain::AssetRecord{
            domain::AssetId{reinterpret_cast<const char*>(raw_id)},
            std::string_view{reinterpret_cast<const char*>(raw_kind)} == "video"
                ? domain::AssetKind::video
                : domain::AssetKind::image,
            reinterpret_cast<const char*>(raw_hash),
            reinterpret_cast<const char*>(raw_path),
            static_cast<std::uint64_t>(sqlite3_column_int64(statement.get(), 4)),
        });
    }
    if (result != SQLITE_DONE) {
        return domain::Result<std::vector<domain::AssetRecord>>::fail(
            {domain::ErrorCode::storage_failure, "Could not iterate Asset listing"});
    }
    return domain::Result<std::vector<domain::AssetRecord>>::ok(std::move(assets));
}

domain::Result<domain::AssetRecord> SqliteContentRepository::load_asset(
    const domain::AssetId& id) const {
    if (id.empty()) {
        return domain::Result<domain::AssetRecord>::fail(
            {domain::ErrorCode::invalid_input, "An asset id is required"});
    }
    Statement statement{store_.native_handle(),
                        "SELECT kind,sha256,relative_path,size_bytes FROM assets "
                        "WHERE id=?1 AND archived=0 LIMIT 1;"};
    if (!statement.ready()) {
        return domain::Result<domain::AssetRecord>::fail(
            {domain::ErrorCode::storage_failure, "Could not prepare Asset load"});
    }
    if (const auto bound = bind_text(statement, 1, id.value); !bound) {
        return domain::Result<domain::AssetRecord>::fail(bound.error());
    }
    if (sqlite3_step(statement.get()) != SQLITE_ROW) {
        return domain::Result<domain::AssetRecord>::fail(
            {domain::ErrorCode::not_found, "Asset was not found"});
    }
    const auto* raw_kind = sqlite3_column_text(statement.get(), 0);
    const auto* raw_hash = sqlite3_column_text(statement.get(), 1);
    const auto* raw_path = sqlite3_column_text(statement.get(), 2);
    if (raw_kind == nullptr || raw_hash == nullptr || raw_path == nullptr) {
        return domain::Result<domain::AssetRecord>::fail(
            {domain::ErrorCode::storage_failure, "Asset contains NULL metadata"});
    }
    return domain::Result<domain::AssetRecord>::ok(domain::AssetRecord{
        id,
        std::string_view{reinterpret_cast<const char*>(raw_kind)} == "video"
            ? domain::AssetKind::video
            : domain::AssetKind::image,
        reinterpret_cast<const char*>(raw_hash),
        reinterpret_cast<const char*>(raw_path),
        static_cast<std::uint64_t>(sqlite3_column_int64(statement.get(), 3)),
    });
}

domain::Status SqliteContentRepository::archive_song(const domain::SongId& id) {
    return archive_content(store_, "songs", "song", id.value);
}

domain::Status SqliteContentRepository::archive_deck(const domain::SermonId& id) {
    return archive_content(store_, "sermon_decks", "sermon", id.value);
}

domain::Status SqliteContentRepository::ensure_service_order(
    const domain::ServiceOrderId& id, std::string title) {
    if (id.empty() || title.empty()) {
        return domain::Status::fail(
            {domain::ErrorCode::invalid_input, "A service order needs a non-empty id and title"});
    }
    return store_.transaction([&](sqlite3* database) {
        Statement statement{database,
                            "INSERT INTO service_orders(id,title) VALUES(?1,?2) "
                            "ON CONFLICT(id) DO UPDATE SET title=excluded.title;"};
        if (!statement.ready()) {
            return sqlite_failure(database, "Could not prepare service-order save");
        }
        if (const auto bound = bind_text(statement, 1, id.value); !bound) {
            return bound;
        }
        if (const auto bound = bind_text(statement, 2, title); !bound) {
            return bound;
        }
        return step_done(statement, "Service-order save failed");
    });
}

domain::Result<domain::ServiceOrderItem> SqliteContentRepository::append_to_order(
    const domain::ServiceOrderId& order_id, domain::ContentRef content) {
    if (order_id.empty()) {
        return domain::Result<domain::ServiceOrderItem>::fail(
            {domain::ErrorCode::invalid_input, "A service-order id is required"});
    }

    std::optional<domain::ServiceOrderItem> item;
    const auto status = store_.transaction([&](sqlite3* database) {
        Statement order_lookup{database, "SELECT 1 FROM service_orders WHERE id=?1 LIMIT 1;"};
        if (!order_lookup.ready()) {
            return sqlite_failure(database, "Could not prepare service-order lookup");
        }
        if (const auto bound = bind_text(order_lookup, 1, order_id.value); !bound) {
            return bound;
        }
        if (sqlite3_step(order_lookup.get()) != SQLITE_ROW) {
            return domain::Status::fail({domain::ErrorCode::not_found, "Service order was not found"});
        }

        const auto exists = content_exists(database, content);
        if (!exists) {
            return domain::Status::fail(exists.error());
        }
        if (!*exists) {
            return domain::Status::fail({domain::ErrorCode::not_found, "Reusable content was not found"});
        }

        Statement ordinal_lookup{database,
                                 "SELECT COALESCE(MAX(ordinal), -1) + 1 FROM service_order_items "
                                 "WHERE service_order_id=?1;"};
        if (!ordinal_lookup.ready()) {
            return sqlite_failure(database, "Could not prepare service-order ordinal lookup");
        }
        if (const auto bound = bind_text(ordinal_lookup, 1, order_id.value); !bound) {
            return bound;
        }
        if (sqlite3_step(ordinal_lookup.get()) != SQLITE_ROW) {
            return sqlite_failure(database, "Could not read service-order ordinal");
        }
        const auto ordinal = sqlite3_column_int64(ordinal_lookup.get(), 0);
        const auto item_id = domain::ServiceOrderItemId{
            order_id.value + "-item-" + std::to_string(ordinal)};
        const auto kind = content.kind == domain::ContentKind::song ? "song" : "sermon";
        const auto content_id = content.kind == domain::ContentKind::song
                                    ? std::get<domain::SongId>(content.id).value
                                    : std::get<domain::SermonId>(content.id).value;

        Statement insert{database,
                         "INSERT INTO service_order_items(id,service_order_id,content_kind,content_id,ordinal) "
                         "VALUES(?1,?2,?3,?4,?5);"};
        if (!insert.ready()) {
            return sqlite_failure(database, "Could not prepare service-order item save");
        }
        if (const auto bound = bind_text(insert, 1, item_id.value); !bound) {
            return bound;
        }
        if (const auto bound = bind_text(insert, 2, order_id.value); !bound) {
            return bound;
        }
        if (const auto bound = bind_text(insert, 3, kind); !bound) {
            return bound;
        }
        if (const auto bound = bind_text(insert, 4, content_id); !bound) {
            return bound;
        }
        if (const auto bound = bind_integer(insert, 5, ordinal); !bound) {
            return bound;
        }
        if (const auto stepped = step_done(insert, "Service-order item save failed"); !stepped) {
            return stepped;
        }
        item = domain::ServiceOrderItem{item_id, std::move(content), ordinal};
        return domain::success();
    });

    if (!status) {
        return domain::Result<domain::ServiceOrderItem>::fail(status.error());
    }
    return domain::Result<domain::ServiceOrderItem>::ok(std::move(*item));
}

domain::Status SqliteContentRepository::remove_from_order(const domain::ServiceOrderItemId& item_id) {
    return store_.transaction([&](sqlite3* database) {
        Statement statement{database, "DELETE FROM service_order_items WHERE id=?1;"};
        if (!statement.ready()) {
            return sqlite_failure(database, "Could not prepare service-order item delete");
        }
        if (const auto bound = bind_text(statement, 1, item_id.value); !bound) {
            return bound;
        }
        if (const auto stepped = step_done(statement, "Service-order item delete failed"); !stepped) {
            return stepped;
        }
        if (sqlite3_changes(database) == 0) {
            return domain::Status::fail({domain::ErrorCode::not_found, "Service-order item was not found"});
        }
        return domain::success();
    });
}

domain::Result<std::vector<domain::ServiceOrderItem>> SqliteContentRepository::load_order(
    const domain::ServiceOrderId& order_id) const {
    Statement statement{store_.native_handle(),
                        "SELECT id,content_kind,content_id,ordinal FROM service_order_items "
                        "WHERE service_order_id=?1 ORDER BY ordinal ASC, id ASC;"};
    if (!statement.ready()) {
        return domain::Result<std::vector<domain::ServiceOrderItem>>::fail(
            {domain::ErrorCode::storage_failure, "Could not prepare service-order load"});
    }
    if (const auto bound = bind_text(statement, 1, order_id.value); !bound) {
        return domain::Result<std::vector<domain::ServiceOrderItem>>::fail(bound.error());
    }

    std::vector<domain::ServiceOrderItem> items;
    while (sqlite3_step(statement.get()) == SQLITE_ROW) {
        const auto* raw_item_id = sqlite3_column_text(statement.get(), 0);
        const auto* raw_kind = sqlite3_column_text(statement.get(), 1);
        const auto* raw_content_id = sqlite3_column_text(statement.get(), 2);
        if (raw_item_id == nullptr || raw_kind == nullptr || raw_content_id == nullptr) {
            return domain::Result<std::vector<domain::ServiceOrderItem>>::fail(
                {domain::ErrorCode::storage_failure, "Service-order row contains NULL text"});
        }
        const std::string kind{reinterpret_cast<const char*>(raw_kind)};
        const std::string content_id{reinterpret_cast<const char*>(raw_content_id)};
        domain::ContentRef content = kind == "song"
                                          ? domain::ContentRef::song(content_id)
                                          : domain::ContentRef::sermon(content_id);
        if (kind != "song" && kind != "sermon") {
            return domain::Result<std::vector<domain::ServiceOrderItem>>::fail(
                {domain::ErrorCode::storage_failure, "Service-order row has an unknown content kind"});
        }
        items.push_back(domain::ServiceOrderItem{
            domain::ServiceOrderItemId{reinterpret_cast<const char*>(raw_item_id)},
            std::move(content),
            sqlite3_column_int64(statement.get(), 3),
        });
    }
    return domain::Result<std::vector<domain::ServiceOrderItem>>::ok(std::move(items));
}

domain::Result<std::size_t> SqliteContentRepository::song_count() const {
    Statement statement{store_.native_handle(), "SELECT COUNT(*) FROM songs WHERE archived=0;"};
    if (!statement.ready()) {
        return domain::Result<std::size_t>::fail(
            {domain::ErrorCode::storage_failure, "Could not prepare Song count"});
    }
    if (sqlite3_step(statement.get()) != SQLITE_ROW) {
        return domain::Result<std::size_t>::fail(
            {domain::ErrorCode::storage_failure, "Could not read Song count"});
    }
    return domain::Result<std::size_t>::ok(static_cast<std::size_t>(sqlite3_column_int64(statement.get(), 0)));
}

}  // namespace shekinah::storage
