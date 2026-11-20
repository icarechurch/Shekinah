#include "shekinah/media/image_cache.hpp"

namespace shekinah::media {

void ImageCache::put(domain::AssetId asset, ImageBytes image) {
    const auto key = std::move(asset.value);
    if (image.bytes > byte_cap_) {
        return;
    }

    if (const auto existing = entries_.find(key); existing != entries_.end()) {
        resident_bytes_ -= existing->second.image.bytes;
        lru_.erase(existing->second.lru_position);
        entries_.erase(existing);
    }

    lru_.push_back(key);
    entries_.emplace(key, Entry{image, std::prev(lru_.end())});
    resident_bytes_ += image.bytes;
    evict_until_fit();
}

bool ImageCache::contains(const domain::AssetId& asset) const {
    return entries_.find(asset.value) != entries_.end();
}

void ImageCache::clear() noexcept {
    entries_.clear();
    lru_.clear();
    resident_bytes_ = 0;
}

void ImageCache::evict_until_fit() {
    while (resident_bytes_ > byte_cap_ && !lru_.empty()) {
        const auto key = lru_.front();
        lru_.pop_front();
        const auto entry = entries_.find(key);
        if (entry != entries_.end()) {
            resident_bytes_ -= entry->second.image.bytes;
            entries_.erase(entry);
        }
    }
}

}  // namespace shekinah::media
