#include "shekinah/storage/project_store.hpp"

#include "project_store_schema.hpp"

namespace shekinah::storage {

domain::Status ProjectStore::migrate() {
    if (const auto status = execute("BEGIN IMMEDIATE;"); !status) {
        return status;
    }
    if (const auto status = execute(project_store_internal::kSchema); !status) {
        execute("ROLLBACK;");
        return status;
    }
    if (schema_version() < 1) {
        if (const auto status = execute("INSERT OR IGNORE INTO schema_migrations(version) VALUES(1);");
            !status) {
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

}  // namespace shekinah::storage
