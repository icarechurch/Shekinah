#include "shekinah/storage/project_store.hpp"

#include <string>
#include <utility>

namespace shekinah::storage {

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
