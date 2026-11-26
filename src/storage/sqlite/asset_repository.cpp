#include "shekinah/storage/content_repository.hpp"

#include "internal/sqlite_support.hpp"

#include <string_view>
#include <utility>
#include <vector>

namespace shekinah::storage {
namespace {

using sqlite_internal::Statement;
using sqlite_internal::bind_text;

}  // namespace

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

}  // namespace shekinah::storage
