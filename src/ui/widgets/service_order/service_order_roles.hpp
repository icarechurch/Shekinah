#pragma once

#include <QtCore/Qt>

namespace shekinah::ui::service_order_internal {

inline constexpr int kKindRole = Qt::UserRole + 1;
inline constexpr int kSlideCountRole = Qt::UserRole + 2;
inline constexpr int kHasBgRole = Qt::UserRole + 3;
inline constexpr int kIsLiveRole = Qt::UserRole + 4;
inline constexpr int kOrdinalRole = Qt::UserRole + 5;
inline constexpr int kDropTargetRole = Qt::UserRole + 6;
inline constexpr int kRowHeight = 46;

}  // namespace shekinah::ui::service_order_internal
