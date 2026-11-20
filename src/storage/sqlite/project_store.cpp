#include "shekinah/storage/project_store.hpp"

#include <utility>

namespace shekinah::storage {

namespace {

constexpr char kSchema[] = R"sql(
CREATE TABLE IF NOT EXISTS schema_migrations(
    version INTEGER PRIMARY KEY,
    applied_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP
);
CREATE TABLE IF NOT EXISTS projects(
    id TEXT PRIMARY KEY,
    title TEXT NOT NULL,
    created_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP
);
CREATE TABLE IF NOT EXISTS assets(
    id TEXT PRIMARY KEY,
    kind TEXT NOT NULL,
    sha256 TEXT NOT NULL UNIQUE,
    relative_path TEXT NOT NULL,
    width INTEGER NOT NULL DEFAULT 0,
    height INTEGER NOT NULL DEFAULT 0,
    duration_ms INTEGER NOT NULL DEFAULT 0,
    archived INTEGER NOT NULL DEFAULT 0
);
CREATE TABLE IF NOT EXISTS songs(
    id TEXT PRIMARY KEY,
    title TEXT NOT NULL,
    default_background_asset_id TEXT REFERENCES assets(id) ON DELETE SET NULL,
    archived INTEGER NOT NULL DEFAULT 0
);
CREATE TABLE IF NOT EXISTS song_reflow_documents(
    song_id TEXT PRIMARY KEY REFERENCES songs(id) ON DELETE CASCADE,
    utf8_text TEXT NOT NULL
);
CREATE TABLE IF NOT EXISTS song_slide_breaks(
    song_id TEXT NOT NULL REFERENCES songs(id) ON DELETE CASCADE,
    ordinal INTEGER NOT NULL,
    utf8_offset INTEGER NOT NULL,
    section_label TEXT NOT NULL DEFAULT '',
    PRIMARY KEY(song_id, ordinal)
);
CREATE TABLE IF NOT EXISTS song_slides(
    id TEXT PRIMARY KEY,
    song_id TEXT NOT NULL REFERENCES songs(id) ON DELETE CASCADE,
    ordinal INTEGER NOT NULL,
    section_label TEXT NOT NULL DEFAULT '',
    lines_json TEXT NOT NULL,
    background_asset_id TEXT REFERENCES assets(id) ON DELETE SET NULL,
    UNIQUE(song_id, ordinal)
);
CREATE TABLE IF NOT EXISTS sermon_decks(
    id TEXT PRIMARY KEY,
    title TEXT NOT NULL,
    archived INTEGER NOT NULL DEFAULT 0
);
CREATE TABLE IF NOT EXISTS sermon_slides(
    id TEXT PRIMARY KEY,
    sermon_id TEXT NOT NULL REFERENCES sermon_decks(id) ON DELETE CASCADE,
    ordinal INTEGER NOT NULL,
    image_asset_id TEXT NOT NULL REFERENCES assets(id),
    UNIQUE(sermon_id, ordinal)
);
CREATE TABLE IF NOT EXISTS service_orders(
    id TEXT PRIMARY KEY,
    title TEXT NOT NULL,
    created_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP
);
CREATE TABLE IF NOT EXISTS service_order_items(
    id TEXT PRIMARY KEY,
    service_order_id TEXT NOT NULL REFERENCES service_orders(id) ON DELETE CASCADE,
    content_kind TEXT NOT NULL,
    content_id TEXT NOT NULL,
    ordinal INTEGER NOT NULL
);
CREATE TABLE IF NOT EXISTS slide_backgrounds(
    slide_id TEXT PRIMARY KEY,
    asset_id TEXT NOT NULL REFERENCES assets(id)
);
CREATE TABLE IF NOT EXISTS settings(
    key TEXT PRIMARY KEY,
    value TEXT NOT NULL
);
)sql";

}  // namespace

ProjectStore::ProjectStore(std::filesystem::path root, sqlite3* database)
    : root_(std::move(root)), database_(database) {}

ProjectStore::ProjectStore(ProjectStore&& other) noexcept
    : root_(std::move(other.root_)), database_(std::exchange(other.database_, nullptr)) {}

ProjectStore& ProjectStore::operator=(ProjectStore&& other) noexcept {
    if (this != &other) {
        close();
        root_ = std::move(other.root_);
        database_ = std::exchange(other.database_, nullptr);
    }
    return *this;
}

ProjectStore::~ProjectStore() { close(); }

domain::Result<ProjectStore> ProjectStore::create_or_open(const std::filesystem::path& root) {
    std::error_code error;
    std::filesystem::create_directories(root / "assets", error);
    if (error) {
        return domain::Result<ProjectStore>::fail(
            {domain::ErrorCode::io_failure, error.message()});
    }

    sqlite3* database = nullptr;
    const auto database_path = (root / "shekinah.sqlite").string();
    if (sqlite3_open_v2(database_path.c_str(), &database,
                        SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX,
                        nullptr) != SQLITE_OK) {
        const auto message = database == nullptr ? "database open failed" : sqlite3_errmsg(database);
        if (database != nullptr) {
            sqlite3_close(database);
        }
        return domain::Result<ProjectStore>::fail(
            {domain::ErrorCode::storage_failure, message});
    }

    ProjectStore store{root, database};
    if (const auto foreign_keys = store.execute("PRAGMA foreign_keys = ON;"); !foreign_keys) {
        return domain::Result<ProjectStore>::fail(foreign_keys.error());
    }
    if (const auto migration = store.migrate(); !migration) {
        return domain::Result<ProjectStore>::fail(migration.error());
    }
    return domain::Result<ProjectStore>::ok(std::move(store));
}

domain::Status ProjectStore::migrate() {
    if (const auto status = execute("BEGIN IMMEDIATE;"); !status) {
        return status;
    }
    if (const auto status = execute(kSchema); !status) {
        execute("ROLLBACK;");
        return status;
    }
    if (schema_version() < 1) {
        if (const auto status = execute("INSERT OR IGNORE INTO schema_migrations(version) VALUES(1);"); !status) {
            execute("ROLLBACK;");
            return status;
        }
    }
    if (schema_version() < 2) {
        if (const auto status = execute(
                "ALTER TABLE assets ADD COLUMN size_bytes INTEGER NOT NULL DEFAULT 0;");
            !status) {
            execute("ROLLBACK;");
            return status;
        }
        if (const auto status = execute("INSERT OR IGNORE INTO schema_migrations(version) VALUES(2);");
            !status) {
            execute("ROLLBACK;");
            return status;
        }
    }
    if (const auto status = execute("COMMIT;"); !status) {
        execute("ROLLBACK;");
        return status;
    }
    return domain::success();
}

domain::Status ProjectStore::transaction(
    const std::function<domain::Status(sqlite3*)>& operation) {
    if (const auto status = execute("BEGIN IMMEDIATE;"); !status) {
        return status;
    }
    const auto result = operation(database_);
    if (!result) {
        execute("ROLLBACK;");
        return result;
    }
    if (const auto commit = execute("COMMIT;"); !commit) {
        execute("ROLLBACK;");
        return commit;
    }
    return domain::success();
}

int ProjectStore::schema_version() const {
    sqlite3_stmt* statement = nullptr;
    if (sqlite3_prepare_v2(database_, "SELECT COALESCE(MAX(version), 0) FROM schema_migrations;", -1,
                           &statement, nullptr) != SQLITE_OK) {
        return 0;
    }
    const auto result = sqlite3_step(statement) == SQLITE_ROW ? sqlite3_column_int(statement, 0) : 0;
    sqlite3_finalize(statement);
    return result;
}

bool ProjectStore::table_exists(std::string_view name) const {
    sqlite3_stmt* statement = nullptr;
    constexpr char query[] = "SELECT 1 FROM sqlite_master WHERE type='table' AND name=?1 LIMIT 1;";
    if (sqlite3_prepare_v2(database_, query, -1, &statement, nullptr) != SQLITE_OK) {
        return false;
    }
    sqlite3_bind_text(statement, 1, name.data(), static_cast<int>(name.size()), SQLITE_TRANSIENT);
    const auto exists = sqlite3_step(statement) == SQLITE_ROW;
    sqlite3_finalize(statement);
    return exists;
}

domain::Error ProjectStore::sqlite_error(sqlite3* database, domain::ErrorCode code) {
    return {code, database == nullptr ? "SQLite error" : sqlite3_errmsg(database)};
}

domain::Status ProjectStore::execute(const char* sql) const {
    char* message = nullptr;
    const auto result = sqlite3_exec(database_, sql, nullptr, nullptr, &message);
    if (result != SQLITE_OK) {
        const std::string text = message == nullptr ? sqlite3_errmsg(database_) : message;
        sqlite3_free(message);
        return domain::Status::fail({domain::ErrorCode::storage_failure, text});
    }
    return domain::success();
}

void ProjectStore::close() noexcept {
    if (database_ != nullptr) {
        sqlite3_close(database_);
        database_ = nullptr;
    }
}

}  // namespace shekinah::storage
