#include "shekinah/storage/asset_importer.hpp"

#include "internal/asset_importer_file.hpp"

#include <string_view>
#include <utility>

namespace shekinah::storage {

domain::Result<domain::AssetRecord> AssetImporter::import_file(
    const std::filesystem::path& source) {
    if (!std::filesystem::is_regular_file(source)) {
        return domain::Result<domain::AssetRecord>::fail(
            {domain::ErrorCode::io_failure, "asset source is not a regular file"});
    }
    const auto classification = asset_importer_internal::classify(source);
    if (!classification) {
        return domain::Result<domain::AssetRecord>::fail(classification.error());
    }
    const auto hash = asset_importer_internal::sha256_file(source);
    if (!hash) {
        return domain::Result<domain::AssetRecord>::fail(hash.error());
    }

    const auto extension = classification.value().second;
    const auto relative = std::filesystem::path{"assets"} / (hash.value() + extension);
    const auto destination = store_.root() / relative;
    if (!std::filesystem::exists(destination)) {
        std::error_code copy_error;
        std::filesystem::copy_file(source, destination,
                                   std::filesystem::copy_options::overwrite_existing, copy_error);
        if (copy_error) {
            return domain::Result<domain::AssetRecord>::fail(
                {domain::ErrorCode::copy_failed, copy_error.message()});
        }
    }

    const auto id = domain::AssetId{"asset-" + hash.value()};
    domain::AssetRecord record{
        id,
        classification.value().first,
        hash.value(),
        relative.string(),
        static_cast<std::uint64_t>(std::filesystem::file_size(source)),
    };
    const auto status = store_.transaction([&](sqlite3* db) {
        sqlite3_stmt* lookup = nullptr;
        constexpr char lookup_sql[] =
            "SELECT id, kind, relative_path, size_bytes FROM assets WHERE sha256=?1 LIMIT 1;";
        if (sqlite3_prepare_v2(db, lookup_sql, -1, &lookup, nullptr) != SQLITE_OK) {
            return domain::Status::fail({domain::ErrorCode::storage_failure, sqlite3_errmsg(db)});
        }
        sqlite3_bind_text(lookup, 1, hash->c_str(), -1, SQLITE_TRANSIENT);
        const auto found = sqlite3_step(lookup) == SQLITE_ROW;
        if (found) {
            const auto* stored_id = reinterpret_cast<const char*>(sqlite3_column_text(lookup, 0));
            const auto* stored_kind = reinterpret_cast<const char*>(sqlite3_column_text(lookup, 1));
            const auto* stored_path = reinterpret_cast<const char*>(sqlite3_column_text(lookup, 2));
            record.id = domain::AssetId{stored_id == nullptr ? "" : stored_id};
            record.kind = stored_kind != nullptr && std::string_view{stored_kind} == "video"
                              ? domain::AssetKind::video
                              : domain::AssetKind::image;
            record.relative_path = stored_path == nullptr ? record.relative_path : stored_path;
            record.size_bytes = static_cast<std::uint64_t>(sqlite3_column_int64(lookup, 3));
            sqlite3_finalize(lookup);
            return domain::success();
        }
        sqlite3_finalize(lookup);

        sqlite3_stmt* insert = nullptr;
        constexpr char insert_sql[] =
            "INSERT INTO assets(id, kind, sha256, relative_path, size_bytes) "
            "VALUES(?1, ?2, ?3, ?4, ?5);";
        if (sqlite3_prepare_v2(db, insert_sql, -1, &insert, nullptr) != SQLITE_OK) {
            return domain::Status::fail({domain::ErrorCode::storage_failure, sqlite3_errmsg(db)});
        }
        sqlite3_bind_text(insert, 1, record.id.value.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(insert, 2, asset_importer_internal::kind_name(record.kind), -1,
                          SQLITE_TRANSIENT);
        sqlite3_bind_text(insert, 3, record.sha256.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(insert, 4, record.relative_path.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int64(insert, 5, static_cast<sqlite3_int64>(record.size_bytes));
        const auto result = sqlite3_step(insert);
        sqlite3_finalize(insert);
        if (result != SQLITE_DONE) {
            return domain::Status::fail({domain::ErrorCode::storage_failure, sqlite3_errmsg(db)});
        }
        return domain::success();
    });
    if (!status) {
        return domain::Result<domain::AssetRecord>::fail(status.error());
    }

    return domain::Result<domain::AssetRecord>::ok(std::move(record));
}

}  // namespace shekinah::storage
