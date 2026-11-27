#pragma once

#include "shekinah/domain/model.hpp"

#include <QImage>
#include <QPointF>
#include <QRect>
#include <QString>
#include <QTextLayout>

#include <memory>

class QPainter;

namespace shekinah::media {

class SlideBackgroundRenderer;

enum class SlideTextMode {
    preview,
    audience,
};

struct SlideRenderData {
    QString text;
    QString background_path;
    domain::FitMode background_fit{domain::FitMode::cover};
    bool show_text{true};
    bool checkerboard_fallback{true};
    SlideTextMode text_mode{SlideTextMode::preview};
};

class SlideRenderer final {
public:
    SlideRenderer();
    ~SlideRenderer();

    SlideRenderer(const SlideRenderer&) = delete;
    SlideRenderer& operator=(const SlideRenderer&) = delete;
    SlideRenderer(SlideRenderer&&) noexcept;
    SlideRenderer& operator=(SlideRenderer&&) noexcept;

    void set_content(SlideRenderData content);
    void paint(QPainter& painter, const QRect& viewport);

private:
    void ensure_text_layout();

    SlideRenderData content_;
    std::unique_ptr<SlideBackgroundRenderer> background_;
    std::shared_ptr<QTextLayout> text_layout_;
    QPointF text_draw_origin_;
    bool text_fits_height_{true};
    bool text_layout_ready_{false};
};

}  // namespace shekinah::media
