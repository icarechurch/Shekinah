#pragma once

#include <QPointF>

namespace shekinah::media::slide_renderer_internal {

inline constexpr int kCanonicalWidth = 1920;
inline constexpr int kCanonicalHeight = 1080;
inline constexpr int kCanonicalPadX = kCanonicalWidth / 24;
inline constexpr int kCanonicalPadY = kCanonicalHeight / 20;
inline constexpr int kCanonicalMinFontPx = 24;
inline constexpr int kCanonicalMaxFontPx = kCanonicalHeight / 8;
inline constexpr QPointF kCanonicalShadowOffset{5.0, 5.0};

}  // namespace shekinah::media::slide_renderer_internal
