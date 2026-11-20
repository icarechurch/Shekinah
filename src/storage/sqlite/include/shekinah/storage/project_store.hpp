#pragma once

#include <filesystem>
#include <functional>
#include <string_view>

#include <sqlite3.h>

#include "shekinah/domain/result.hpp"

namespace shekinah::storage {

class ProjectStore {
public:
    static domain::Result<ProjectStore> create_or_open(const std::filesystem::path& root);

    ProjectStore(const ProjectStore&) = delete;
    ProjectStore& operator=(const ProjectStore&) = delete;
    ProjectStore(ProjectStore&& other) noexcept;
    ProjectStore& operator=(ProjectStore&& other) noexcept;
    ~ProjectStore();

    domain::Status migrate();
    domain::Status transaction(const std::function<domain::Status(sqlite3*)>& operation);

    [[nodiscard]] int schema_version() const;
    [[nodiscard]] bool table_exists(std::string_view name) const;
    [[nodiscard]] sqlite3* native_handle() const noexcept { return database_; }
    [[nodiscard]] const std::filesystem::path& root() const noexcept { return root_; }

private:
    ProjectStore(std::filesystem::path root, sqlite3* database);
    static domain::Error sqlite_error(sqlite3* database, domain::ErrorCode code);
    domain::Status execute(const char* sql) const;
    void close() noexcept;

    std::filesystem::path root_;
    sqlite3* database_{nullptr};
};

}  // namespace shekinah::storage
