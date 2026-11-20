#include "shekinah/storage/asset_importer.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <fstream>
#include <iomanip>
#include <openssl/evp.h>
#include <sstream>
#include <string_view>
#include <utility>

namespace shekinah::storage {

namespace {

domain::Result<std::pair<domain::AssetKind, std::string>> classify(
    const std::filesystem::path& source) {
    auto extension = source.extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(),
                   [](unsigned char value) { return static_cast<char>(std::tolower(value)); });
    if (extension == ".png" || extension == ".jpg" || extension == ".jpeg") {
        return domain::Result<std::pair<domain::AssetKind, std::string>>::ok(
            {domain::AssetKind::image, extension});
    }
    if (extension == ".mp4") {
        return domain::Result<std::pair<domain::AssetKind, std::string>>::ok(
            {domain::AssetKind::video, extension});
    }
    return domain::Result<std::pair<domain::AssetKind, std::string>>::fail(
        {domain::ErrorCode::unsupported_media, "supported assets are PNG, JPG, JPEG, and MP4"});
}

domain::Result<std::string> sha256_file(const std::filesystem::path& source) {
    std::ifstream input(source, std::ios::binary);
    if (!input) {
        return domain::Result<std::string>::fail(
            {domain::ErrorCode::io_failure, "unable to open asset"});
    }

    EVP_MD_CTX* context = EVP_MD_CTX_new();
    if (context == nullptr || EVP_DigestInit_ex(context, EVP_sha256(), nullptr) != 1) {
        EVP_MD_CTX_free(context);
        return domain::Result<std::string>::fail(
            {domain::ErrorCode::storage_failure, "unable to initialize SHA-256"});
    }

    std::array<char, 64 * 1024> buffer{};
    while (input.read(buffer.data(), static_cast<std::streamsize>(buffer.size())) || input.gcount() > 0) {
        const auto count = input.gcount();
        if (EVP_DigestUpdate(context, buffer.data(), static_cast<std::size_t>(count)) != 1) {
            EVP_MD_CTX_free(context);
            return domain::Result<std::string>::fail(
                {domain::ErrorCode::storage_failure, "unable to hash asset"});
        }
    }

    std::array<unsigned char, EVP_MAX_MD_SIZE> digest{};
    unsigned int length = 0;
    if (EVP_DigestFinal_ex(context, digest.data(), &length) != 1) {
        EVP_MD_CTX_free(context);
        return domain::Result<std::string>::fail(
            {domain::ErrorCode::storage_failure, "unable to finalize asset hash"});
    }
    EVP_MD_CTX_free(context);

    std::ostringstream output;
    output << std::hex << std::setfill('0');
    for (unsigned int index = 0; index < length; ++index) {
        output << std::setw(2) << static_cast<unsigned int>(digest[index]);
    }
    return domain::Result<std::string>::ok(output.str());
}

const char* kind_name(domain::AssetKind kind) {
    return kind == domain::AssetKind::image ? "image" : "video";
}

}  // namespace

domain::Result<domain::AssetRecord> AssetImporter::import_file(
    const std::filesystem::path& source) {
    if (!std::filesystem::is_regular_file(source)) {
        return domain::Result<domain::AssetRecord>::fail(
            {domain::ErrorCode::io_failure, "asset source is not a regular file"});
    }
    const auto classification = classify(source);
    if (!classification) {
        return domain::Result<domain::AssetRecord>::fail(classification.error());
    }
    const auto hash = sha256_file(source);
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
        constexpr char lookup_sql[] = "SELECT id, kind, relative_path FROM assets WHERE sha256=?1 LIMIT 1;";
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
            sqlite3_finalize(lookup);
            return domain::success();
        }
        sqlite3_finalize(lookup);

        sqlite3_stmt* insert = nullptr;
        constexpr char insert_sql[] =
            "INSERT INTO assets(id, kind, sha256, relative_path) VALUES(?1, ?2, ?3, ?4);";
        if (sqlite3_prepare_v2(db, insert_sql, -1, &insert, nullptr) != SQLITE_OK) {
            return domain::Status::fail({domain::ErrorCode::storage_failure, sqlite3_errmsg(db)});
        }
        sqlite3_bind_text(insert, 1, record.id.value.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(insert, 2, kind_name(record.kind), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(insert, 3, record.sha256.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(insert, 4, record.relative_path.c_str(), -1, SQLITE_TRANSIENT);
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
