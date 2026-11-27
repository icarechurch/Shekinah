#include "shekinah/ui/cue_grid_widget.hpp"

#include "cue_grid_overlay.hpp"

#include <QFrame>
#include <QScrollArea>
#include <QVBoxLayout>

namespace shekinah::ui {

void CueGridWidget::setup_grid_view(QVBoxLayout* outer_layout) {
    grid_scroll_ = new QScrollArea{this};
    grid_scroll_->setObjectName(QStringLiteral("cue-grid-scroll"));
    grid_scroll_->setFrameShape(QFrame::NoFrame);
    grid_scroll_->setWidgetResizable(true);
    grid_scroll_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    grid_scroll_->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    grid_container_->setObjectName(QStringLiteral("cue-grid-container"));
    grid_container_->setAcceptDrops(true);
    grid_container_->installEventFilter(this);
    insert_overlay_ = new InsertOverlay{grid_container_};
    insert_overlay_->hide();
    grid_scroll_->setWidget(grid_container_);
    grid_scroll_->viewport()->setAcceptDrops(true);
    grid_scroll_->viewport()->installEventFilter(this);
    outer_layout->addWidget(grid_scroll_);
    grid_scroll_->setVisible(false);
}

}  // namespace shekinah::ui
