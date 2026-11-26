#pragma once

#include "shekinah/ui/cue_grid_widget.hpp"

#include "shekinah/ui/support/theme.hpp"

#include <QPainter>

namespace shekinah::ui {

class CueGridWidget::InsertOverlay final : public QWidget {
public:
    explicit InsertOverlay(QWidget* parent) : QWidget(parent) {
        setObjectName(QStringLiteral("cue-insert-overlay"));
        setAttribute(Qt::WA_TransparentForMouseEvents, true);
        setAttribute(Qt::WA_NoSystemBackground, true);
        setAttribute(Qt::WA_TranslucentBackground, true);
    }

    void set_indicator_rect(QRect rect) {
        indicator_rect_ = rect;
        if (indicator_rect_.isValid() && !indicator_rect_.isEmpty()) {
            show();
            raise();
        } else {
            hide();
        }
        update();
    }

protected:
    void paintEvent(QPaintEvent*) override {
        if (!indicator_rect_.isValid() || indicator_rect_.isEmpty()) {
            return;
        }
        const auto& p = ThemeManager::instance().palette();
        QPainter painter{this};
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setPen(Qt::NoPen);
        painter.setBrush(p.drop_highlight);
        painter.drawRoundedRect(indicator_rect_, 2.0, 2.0);
        if (indicator_rect_.height() >= indicator_rect_.width()) {
            const qreal cx = indicator_rect_.center().x() + 0.5;
            painter.drawEllipse(QPointF{cx, static_cast<qreal>(indicator_rect_.top() + 3)}, 4.0, 4.0);
            painter.drawEllipse(QPointF{cx, static_cast<qreal>(indicator_rect_.bottom() - 2)}, 4.0, 4.0);
        } else {
            const qreal cy = indicator_rect_.center().y() + 0.5;
            painter.drawEllipse(QPointF{static_cast<qreal>(indicator_rect_.left() + 3), cy}, 4.0, 4.0);
            painter.drawEllipse(QPointF{static_cast<qreal>(indicator_rect_.right() - 2), cy}, 4.0, 4.0);
        }
    }

private:
    QRect indicator_rect_;
};


}  // namespace shekinah::ui

