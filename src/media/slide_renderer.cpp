#include "shekinah/media/slide_renderer.hpp"

#include "slide_renderer_background.hpp"
#include "slide_renderer_text.hpp"

#include <QPainter>

#include <utility>

namespace shekinah::media {

SlideRenderer::SlideRenderer()
    : background_(std::make_unique<SlideBackgroundRenderer>()),
      text_(std::make_unique<SlideTextRenderer>()) {}

SlideRenderer::~SlideRenderer() = default;

SlideRenderer::SlideRenderer(SlideRenderer&&) noexcept = default;

SlideRenderer& SlideRenderer::operator=(SlideRenderer&&) noexcept = default;

void SlideRenderer::set_content(SlideRenderData content) {
    content_ = std::move(content);
    background_->set_content(
        content_.background_path, content_.background_fit, content_.checkerboard_fallback);
    text_->set_content(content_.text, content_.show_text);
}

void SlideRenderer::paint(QPainter& painter, const QRect& viewport) {
    if (viewport.isEmpty()) {
        return;
    }

    painter.save();
    painter.setClipRect(viewport);
    background_->paint(painter, viewport);
    text_->paint(painter, viewport);
    painter.restore();
}

}  // namespace shekinah::media
