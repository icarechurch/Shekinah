#pragma once

#include <cstddef>
#include <list>
#include <map>

#include "shekinah/domain/model.hpp"

namespace shekinah::media {

struct ImageBytes {
    std::size_t bytes{0};
};

class ImageCache {
public:
    explicit ImageCache(std::size_t byte_cap) : byte_cap_(byte_cap) {}

    void put(domain::AssetId asset, ImageBytes image);
    [[nodiscard]] bool contains(const domain::AssetId& asset) const;
    [[nodiscard]] std::size_t resident_bytes() const noexcept { return resident_bytes_; }
    void clear() noexcept;

private:
    struct Entry {
        ImageBytes image;
        std::list<std::string>::iterator lru_position;
    };

    void evict_until_fit();

    std::size_t byte_cap_;
    std::size_t resident_bytes_{0};
    std::list<std::string> lru_;
    std::map<std::string, Entry> entries_;
};

}  // namespace shekinah::media
