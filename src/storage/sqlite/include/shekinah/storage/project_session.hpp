#pragma once

#include "shekinah/domain/result.hpp"

#include <filesystem>
#include <memory>

namespace shekinah::storage {

class ProjectStore;
class SqliteContentRepository;

class ProjectSession final {
public:
    ProjectSession();
    ProjectSession(const ProjectSession&) = delete;
    ProjectSession& operator=(const ProjectSession&) = delete;
    ProjectSession(ProjectSession&&) = delete;
    ProjectSession& operator=(ProjectSession&&) = delete;
    ~ProjectSession();

    domain::Status open(const std::filesystem::path& root);
    void close() noexcept;

    [[nodiscard]] bool is_open() const noexcept { return store_ != nullptr; }
    [[nodiscard]] const std::filesystem::path& root() const noexcept { return root_; }
    [[nodiscard]] ProjectStore* store() noexcept { return store_.get(); }
    [[nodiscard]] const ProjectStore* store() const noexcept { return store_.get(); }
    [[nodiscard]] SqliteContentRepository* repository() noexcept { return repository_.get(); }
    [[nodiscard]] const SqliteContentRepository* repository() const noexcept {
        return repository_.get();
    }

private:
    std::filesystem::path root_;
    std::unique_ptr<ProjectStore> store_;
    std::unique_ptr<SqliteContentRepository> repository_;
};

}  // namespace shekinah::storage
