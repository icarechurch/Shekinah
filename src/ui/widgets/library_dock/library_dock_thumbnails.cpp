#include "shekinah/ui/library_dock_widget.hpp"

#include "library_components.hpp"

#include <QImageReader>
#include <QPixmap>

#include <algorithm>
#include <utility>

namespace shekinah::ui {
using namespace library_internal;

QIcon LibraryDockWidget::load_bounded_thumbnail(const domain::AssetRecord& asset) {
    const QString cache_key = QStringLiteral("%1|%2|%3|%4|%5")
                                  .arg(QString::fromStdString(asset_root_.string()))
                                  .arg(QString::fromStdString(asset.id.value))
                                  .arg(QString::fromStdString(asset.sha256))
                                  .arg(QString::fromStdString(asset.relative_path))
                                  .arg(static_cast<qulonglong>(asset.size_bytes));
    if (const QIcon* cached = thumbnail_cache_.object(cache_key); cached != nullptr) {
        return *cached;
    }

    QImageReader reader{QString::fromStdString((asset_root_ / asset.relative_path).string())};
    reader.setAutoTransform(true);
    const auto source_size = reader.size();
    if (source_size.isValid()) {
        reader.setScaledSize(
            source_size.scaled(kThumbnailWidth, kThumbnailHeight, Qt::KeepAspectRatio));
    }
    QImage image = reader.read();
    if (image.isNull()) {
        return QIcon{};
    }
    if (image.width() > kThumbnailWidth || image.height() > kThumbnailHeight) {
        image = image.scaled(
            kThumbnailWidth, kThumbnailHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
    QPixmap pixmap = QPixmap::fromImage(image);
    if (pixmap.isNull()) {
        return QIcon{};
    }
    const int cost_kb = std::max(1, (pixmap.width() * pixmap.height() * 4) / 1024);
    auto* cached_icon = new QIcon{std::move(pixmap)};
    QIcon result = *cached_icon;
    thumbnail_cache_.insert(cache_key, cached_icon, cost_kb);
    return result;
}

}  // namespace shekinah::ui
