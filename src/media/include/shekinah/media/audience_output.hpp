#pragma once

#include "shekinah/domain/model.hpp"
#include "shekinah/domain/result.hpp"

#include <QWidget>
#include <QImage>
#include <QString>

#include <filesystem>
class QScreen;
class QMediaPlayer;
class QVideoWidget;
class QLabel;

namespace shekinah::media {

enum class OutputMode { hidden, movable, fullscreen };

class AudienceOutput final : public QWidget {
public:
    explicit AudienceOutput(QWidget* parent = nullptr);

    void show_fullscreen(QScreen* screen);
    void show_movable();
    void present(const domain::ProgramState& program);
    void set_live_text(QString text);
    void set_video_paused(bool paused) noexcept;
    domain::Status load_slide_image(const std::filesystem::path& path);
    void clear_slide_image() noexcept;
    domain::Status load_loop_video(const std::filesystem::path& path);
    void clear_loop_video();

    [[nodiscard]] OutputMode mode() const noexcept { return mode_; }
    [[nodiscard]] const domain::ProgramState& program() const noexcept { return program_; }
    [[nodiscard]] bool video_paused() const noexcept { return video_paused_; }
    [[nodiscard]] bool slide_image_loaded() const noexcept { return slide_image_loaded_; }
    [[nodiscard]] bool loop_video_loaded() const noexcept { return loop_video_loaded_; }
    [[nodiscard]] const QString& live_text() const noexcept { return live_text_; }

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    OutputMode mode_{OutputMode::hidden};
    domain::ProgramState program_{};
    bool video_paused_{false};
    bool slide_image_loaded_{false};
    bool loop_video_loaded_{false};
    QString live_text_;
    QImage slide_image_;
    QMediaPlayer* player_{nullptr};
    QVideoWidget* video_widget_{nullptr};
    QLabel* lyric_overlay_{nullptr};
};

}  // namespace shekinah::media
