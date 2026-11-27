#pragma once

#include "shekinah/domain/result.hpp"

#include <QRect>

#include <filesystem>

class QMediaPlayer;
class QVideoWidget;
class QWidget;

namespace shekinah::media {

class AudienceVideoOutput final {
public:
    explicit AudienceVideoOutput(QWidget* parent);

    void set_geometry(const QRect& geometry);
    void set_visible(bool visible);
    domain::Status load(const std::filesystem::path& path, bool visible);
    void clear() noexcept;
    void set_paused(bool paused) noexcept;

    [[nodiscard]] bool paused() const noexcept { return paused_; }
    [[nodiscard]] bool loaded() const noexcept { return loaded_; }

private:
    QMediaPlayer* player_{nullptr};
    QVideoWidget* widget_{nullptr};
    bool paused_{false};
    bool loaded_{false};
};

}  // namespace shekinah::media
