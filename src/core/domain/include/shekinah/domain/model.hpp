#pragma once

#include <optional>
#include <string>
#include <utility>
#include <variant>

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
