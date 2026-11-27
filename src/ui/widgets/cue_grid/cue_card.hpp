#pragma once

#include "shekinah/media/slide_renderer.hpp"
#include "shekinah/ui/cue_grid_widget.hpp"

#include <QPoint>
#include <QRect>
#include <QSize>

#include <cstddef>

class QDragEnterEvent;
class QDragLeaveEvent;
class QDragMoveEvent;
class QDropEvent;
class QMimeData;
class QMouseEvent;
class QPaintEvent;

namespace shekinah::ui {

class CueGridWidget::CueCard final : public QWidget {
public:
    CueCard(const CueView& cue, std::size_t index, CueGridWidget& owner);

    int heightForWidth(int width) const override;
    bool hasHeightForWidth() const override;
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;
    QRect preview_rect() const;
    void set_live(bool live);

protected:
    enum class DropZone { on_card, insert_before, insert_after };

    DropZone drop_zone_for(QPoint pos) const;
    void update_drag_hover(const QMimeData* mime, QPoint pos);
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragMoveEvent(QDragMoveEvent* event) override;
    void dragLeaveEvent(QDragLeaveEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

private:
    CueView cue_;
    std::size_t index_{0};
    CueGridWidget& owner_;
    media::SlideRenderer slide_renderer_;
};

}  // namespace shekinah::ui
