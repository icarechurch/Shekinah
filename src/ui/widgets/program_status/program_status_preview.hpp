#pragma once

#include "shekinah/media/slide_renderer.hpp"
#include "shekinah/ui/program_status_widget.hpp"

#include <QFrame>

class QPaintEvent;

namespace shekinah::ui {

class ProgramSlidePreview final : public QFrame {
public:
    explicit ProgramSlidePreview(QWidget* parent);

    [[nodiscard]] bool hasHeightForWidth() const override;
    [[nodiscard]] int heightForWidth(int width) const override;

    void set_mode(domain::ProgramMode mode);
    void set_content(QString text, QString background_path, domain::FitMode background_fit);

protected:
    void paintEvent(QPaintEvent*) override;

private:
    domain::ProgramMode mode_{domain::ProgramMode::clear};
    QString text_;
    QString background_path_;
    domain::FitMode background_fit_{domain::FitMode::cover};
    media::SlideRenderer renderer_;
};

[[nodiscard]] QString mode_text(domain::ProgramMode mode);
[[nodiscard]] QString fit_mode_text(domain::FitMode fit);

}  // namespace shekinah::ui
