#pragma once

#include <QPoint>
#include <QPointF>
#include <QtGlobal>

namespace test_support {

#if QT_VERSION >= QT_VERSION_CHECK(6, 12, 0)
using DragEventPosition = QPointF;
#else
using DragEventPosition = QPoint;
#endif

inline DragEventPosition drag_event_position(const QPoint& position) {
    return DragEventPosition{position};
}

}  // namespace test_support
