#pragma once

#include <optional>
#include <cstdint>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace shekinah::domain {

template <typename Tag>
struct StrongId {
    std::string value;

    StrongId() = default;
    explicit StrongId(std::string text) : value(std::move(text)) {}
    explicit StrongId(const char* text) : value(text) {}

    [[nodiscard]] bool empty() const noexcept { return value.empty(); }
    friend bool operator==(const StrongId&, const StrongId&) = default;
};

struct SongTag;
struct SermonTag;
struct AssetTag;
struct ServiceOrderTag;
struct ServiceOrderItemTag;
struct CueTag;

using SongId = StrongId<SongTag>;
using SermonId = StrongId<SermonTag>;
using AssetId = StrongId<AssetTag>;
using ServiceOrderId = StrongId<ServiceOrderTag>;
using ServiceOrderItemId = StrongId<ServiceOrderItemTag>;
using CueId = StrongId<CueTag>;

enum class AssetKind { image, video };

struct AssetRecord {
    AssetId id;
    AssetKind kind;
    std::string sha256;
    std::string relative_path;
    std::uint64_t size_bytes{0};
};

enum class ContentKind { song, sermon };

struct ContentRef {
    ContentKind kind;
    std::variant<SongId, SermonId> id;

    static ContentRef song(std::string value) {
        return {ContentKind::song, SongId{std::move(value)}};
    }

    static ContentRef sermon(std::string value) {
        return {ContentKind::sermon, SermonId{std::move(value)}};
    }

    friend bool operator==(const ContentRef&, const ContentRef&) = default;
};

enum class FitMode { contain, cover, stretch };

struct BackgroundRef {
    AssetId asset;
    FitMode fit{FitMode::cover};
    friend bool operator==(const BackgroundRef&, const BackgroundRef&) = default;
};

struct Slide {
    CueId id;
    std::string section;
    std::vector<std::string> lines;
    std::optional<BackgroundRef> background;
    friend bool operator==(const Slide&, const Slide&) = default;
};

struct Song {
    SongId id;
    std::string title;
    std::optional<BackgroundRef> default_background;
    std::vector<Slide> slides;
};

struct SermonSlide {
    CueId id;
    AssetId image;
    friend bool operator==(const SermonSlide&, const SermonSlide&) = default;
};

struct SermonDeck {
    SermonId id;
    std::string title;
    std::vector<SermonSlide> slides;
};

struct ServiceOrderItem {
    ServiceOrderItemId id;
    ContentRef content;
    std::int64_t ordinal{0};
};

struct CueRef {
    ContentRef content;
    CueId cue;
    friend bool operator==(const CueRef&, const CueRef&) = default;
};

enum class ProgramMode { cue, clear, black };

struct ProgramState {
    ProgramMode mode{ProgramMode::clear};
    std::optional<CueRef> live_cue;
    friend bool operator==(const ProgramState&, const ProgramState&) = default;
};

}  // namespace shekinah::domain
