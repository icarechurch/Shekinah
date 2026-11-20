#include "shekinah/ui/cue_grid_widget.hpp"

#include <QGridLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QPushButton>
#include <QResizeEvent>
#include <QScreen>
#include <QVBoxLayout>

#include <algorithm>

namespace shekinah::ui {

class CueGridWidget::CueButton final : public QPushButton {
public:
    CueButton(const CueView& cue, CueGridWidget& owner) : QPushButton(&owner), cue_(cue), owner_(owner) {
        setObjectName(QStringLiteral("cue-%1").arg(QString::fromStdString(cue.id.value)));
        setMinimumSize(120, 92);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        setCursor(Qt::PointingHandCursor);
        const auto heading = cue.section.isEmpty() ? QString::fromStdString(cue.id.value) : cue.section;
        setText(cue.summary.isEmpty() ? heading : heading + QStringLiteral("\n") + cue.summary);
        setToolTip(cue.summary);
    }

protected:
    void mousePressEvent(QMouseEvent* event) override {
        if (event->button() == Qt::LeftButton) {
            owner_.handle_left_click(cue_.id);
        } else if (event->button() == Qt::RightButton) {
            owner_.handle_right_click(cue_.id, mapToGlobal(event->position().toPoint()));
        }
        QPushButton::mousePressEvent(event);
    }

private:
    CueView cue_;
    CueGridWidget& owner_;
};

CueGridWidget::CueGridWidget(QWidget* parent) : QWidget(parent), grid_container_(new QWidget(this)) {
    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->addWidget(grid_container_);
    setMinimumWidth(240);
}

int CueGridWidget::column_count_for_geometry(int viewport_width, int screen_width) {
    if (viewport_width < 320) {
        return 1;
    }
    if (viewport_width < 520) {
        return 2;
    }
    if (screen_width < 1600) {
        return 3;
    }
    if (screen_width < 2300) {
        return 4;
    }
    if (screen_width < 3200) {
        return 5;
    }
    return 6;
}

void CueGridWidget::set_cues(std::vector<CueView> cues) {
    cues_ = std::move(cues);
    rebuild_layout();
}

QRect CueGridWidget::visual_rect(const domain::CueId& cue) const {
    const auto button = std::find_if(buttons_.begin(), buttons_.end(), [&](const auto* candidate) {
        return candidate->objectName() == QStringLiteral("cue-%1").arg(QString::fromStdString(cue.value));
    });
    if (button == buttons_.end()) {
        return {};
    }
    return QRect(mapFrom(*button, QPoint{0, 0}), (*button)->size());
}

void CueGridWidget::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    rebuild_layout();
}

void CueGridWidget::rebuild_layout() {
    if (grid_container_ == nullptr) {
        return;
    }
    if (auto* old_layout = grid_container_->layout(); old_layout != nullptr) {
        while (auto* item = old_layout->takeAt(0)) {
            if (auto* widget = item->widget(); widget != nullptr) {
                widget->deleteLater();
            }
            delete item;
        }
        delete old_layout;
    }
    buttons_.clear();

    auto* layout = new QGridLayout(grid_container_);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(8);
    const int screen_width = screen() == nullptr ? 1920 : screen()->size().width();
    const int columns = column_count_for_geometry(width(), screen_width);
    for (std::size_t index = 0; index != cues_.size(); ++index) {
        auto* button = new CueButton{cues_[index], *this};
        buttons_.push_back(button);
        const int row = static_cast<int>(index / static_cast<std::size_t>(columns));
        const int column = static_cast<int>(index % static_cast<std::size_t>(columns));
        layout->addWidget(button, row, column);
    }
}

void CueGridWidget::handle_left_click(const domain::CueId& cue) {
    if (take_live_handler_) {
        take_live_handler_(cue);
    }
}

void CueGridWidget::handle_right_click(const domain::CueId& cue, QPoint global_position) {
    if (context_handler_) {
        context_handler_(cue, global_position);
    }
}

}  // namespace shekinah::ui
