#include "shekinah/media/slide_renderer.hpp"

#include <QImageReader>

namespace shekinah::media {

void SlideRenderer::ensure_background_image(const QRect& viewport) {
    if (content_.background_path.isEmpty() || background_image_size_ == viewport.size()) {
        return;
    }
    background_image_size_ = viewport.size();
    background_image_ = {};

    QImageReader reader{content_.background_path};
    reader.setAutoTransform(true);
    QSize decoded_size = viewport.size();
    const auto source_size = reader.size();
    if (source_size.isValid() && content_.background_fit != domain::FitMode::stretch) {
        decoded_size = source_size;
        decoded_size.scale(
            viewport.size(),
            content_.background_fit == domain::FitMode::contain
                ? Qt::KeepAspectRatio
                : Qt::KeepAspectRatioByExpanding);
    }
    reader.setScaledSize(decoded_size);
    background_image_ = reader.read();
}

}  // namespace shekinah::media
