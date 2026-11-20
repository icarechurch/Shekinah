#pragma once

#include "shekinah/domain/model.hpp"

#include <QWidget>

class QScreen;

namespace shekinah::media {

enum class OutputMode { hidden, movable, fullscreen };

class AudienceOutput final : public QWidget {
public:
    explicit AudienceOutput(QWidget* parent = nullptr);

    void show_fullscreen(QScreen* screen);
    void show_movable();
    void present(const domain::ProgramState& program);
    void set_video_paused(bool paused) noexcept;

    [[nodiscard]] OutputMode mode() const noexcept { return mode_; }
    [[nodiscard]] const domain::ProgramState& program() const noexcept { return program_; }
    [[nodiscard]] bool video_paused() const noexcept { return video_paused_; }

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    OutputMode mode_{OutputMode::hidden};
    domain::ProgramState program_{};
    bool video_paused_{false};
};

}  // namespace shekinah::media
