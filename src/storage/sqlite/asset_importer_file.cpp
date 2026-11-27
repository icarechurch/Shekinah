#include "internal/asset_importer_file.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <fstream>
#include <iomanip>
#include <openssl/evp.h>
#include <sstream>

namespace shekinah::storage::asset_importer_internal {

domain::Result<std::pair<domain::AssetKind, std::string>> classify(
    const std::filesystem::path& source) {
    auto extension = source.extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(),
                   [](unsigned char value) { return static_cast<char>(std::tolower(value)); });
    if (extension == ".png" || extension == ".jpg" || extension == ".jpeg" ||
        extension == ".webp" || extension == ".bmp" || extension == ".gif" ||
        extension == ".tif" || extension == ".tiff") {
        return domain::Result<std::pair<domain::AssetKind, std::string>>::ok(
            {domain::AssetKind::image, extension});
    }
    if (extension == ".mp4") {
        return domain::Result<std::pair<domain::AssetKind, std::string>>::ok(
            {domain::AssetKind::video, extension});
    }
    return domain::Result<std::pair<domain::AssetKind, std::string>>::fail(
        {domain::ErrorCode::unsupported_media,
         "supported assets are PNG, JPG, JPEG, WEBP, BMP, GIF, TIFF, and MP4"});
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
    while (input.read(buffer.data(), static_cast<std::streamsize>(buffer.size())) ||
           input.gcount() > 0) {
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

}  // namespace shekinah::storage::asset_importer_internal
