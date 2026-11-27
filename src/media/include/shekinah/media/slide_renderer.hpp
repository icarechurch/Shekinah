#pragma once

#include "shekinah/domain/model.hpp"

#include <QRect>
#include <QString>

#include <memory>

class QPainter;

namespace shekinah::media {

class SlideBackgroundRenderer;
class SlideTextRenderer;

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
    SlideRenderData content_;
    std::unique_ptr<SlideBackgroundRenderer> background_;
    std::unique_ptr<SlideTextRenderer> text_;
};

}  // namespace shekinah::media
