#include "shekinah/storage/project_session.hpp"

#include "shekinah/storage/content_repository.hpp"
#include "shekinah/storage/project_store.hpp"

#include <utility>

namespace shekinah::storage {

ProjectSession::ProjectSession() = default;
ProjectSession::~ProjectSession() = default;

domain::Status ProjectSession::open(const std::filesystem::path& root) {
    close();

    auto opened = ProjectStore::create_or_open(root);
    if (!opened) {
        return domain::Status::fail(opened.error());
    }

    store_ = std::make_unique<ProjectStore>(std::move(opened.value()));
    repository_ = std::make_unique<SqliteContentRepository>(*store_);
    root_ = store_->root();
    return domain::success();
}

void ProjectSession::close() noexcept {
    repository_.reset();
    store_.reset();
    root_.clear();
}

}  // namespace shekinah::storage
