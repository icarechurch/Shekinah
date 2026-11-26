#pragma once

#include "shekinah/domain/result.hpp"

#include <sqlite3.h>

#include <cstdint>
#include <optional>
#include <string>
#include <utility>

namespace shekinah::storage::sqlite_internal {

class Statement final {
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

inline domain::Status sqlite_failure(sqlite3* database, std::string context) {
    context += ": ";
    context += database == nullptr ? "SQLite error" : sqlite3_errmsg(database);
    return domain::Status::fail({domain::ErrorCode::storage_failure, std::move(context)});
}

inline domain::Status bind_text(Statement& statement, int index, const std::string& value) {
    if (sqlite3_bind_text(statement.get(), index, value.c_str(), static_cast<int>(value.size()),
                          SQLITE_TRANSIENT) != SQLITE_OK) {
        return sqlite_failure(statement.database(), "SQLite text bind failed");
    }
    return domain::success();
}

inline domain::Status bind_optional_text(
    Statement& statement, int index, const std::optional<std::string>& value) {
    const int result = value.has_value()
                           ? sqlite3_bind_text(statement.get(), index, value->c_str(),
                                               static_cast<int>(value->size()), SQLITE_TRANSIENT)
                           : sqlite3_bind_null(statement.get(), index);
    if (result != SQLITE_OK) {
        return sqlite_failure(statement.database(), "SQLite optional text bind failed");
    }
    return domain::success();
}

inline domain::Status bind_integer(Statement& statement, int index, std::int64_t value) {
    if (sqlite3_bind_int64(statement.get(), index, value) != SQLITE_OK) {
        return sqlite_failure(statement.database(), "SQLite integer bind failed");
    }
    return domain::success();
}

inline domain::Status step_done(Statement& statement, const char* operation) {
    if (sqlite3_step(statement.get()) != SQLITE_DONE) {
        return sqlite_failure(statement.database(), operation);
    }
    return domain::success();
}

}  // namespace shekinah::storage::sqlite_internal
