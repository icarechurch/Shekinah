#pragma once

#include <QPointF>
#include <QRect>
#include <QString>

#include <memory>

class QPainter;
class QTextLayout;

namespace shekinah::media {

class SlideTextRenderer final {
public:
    void set_content(QString text, bool show_text);
    void paint(QPainter& painter, const QRect& viewport);

private:
    void ensure_text_layout();

    QString text_;
    bool show_text_{true};
    std::shared_ptr<QTextLayout> text_layout_;
    QPointF text_draw_origin_;
    bool text_fits_height_{true};
    bool text_layout_ready_{false};
};

}  // namespace shekinah::media
