#pragma once

#include "shekinah/media/slide_renderer.hpp"
#include "shekinah/ui/program_status_widget.hpp"
#include "shekinah/ui/support/theme.hpp"

#include <QFrame>
#include <QPainter>

#include <algorithm>
#include <cmath>

namespace shekinah::ui {
namespace {

class ProgramSlidePreview final : public QFrame {
public:
    explicit ProgramSlidePreview(QWidget* parent) : QFrame(parent) {
        setAttribute(Qt::WA_OpaquePaintEvent);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        setMinimumHeight(96);
    }

    [[nodiscard]] bool hasHeightForWidth() const override { return true; }

    [[nodiscard]] int heightForWidth(int width) const override {
        return std::clamp(static_cast<int>(std::lround(static_cast<double>(width) * 9.0 / 16.0)),
                          96,
                          190);
    }

    void set_mode(domain::ProgramMode mode) {
        mode_ = mode;
        update();
    }

    void set_content(QString text, QString background_path, domain::FitMode background_fit) {
        text_ = std::move(text);
        background_path_ = std::move(background_path);
        background_fit_ = background_fit;
        renderer_.set_content(media::SlideRenderData{
            text_,
            background_path_,
            background_fit_,
            true,
            false,
        });
        setProperty("slide-text", text_);
        setProperty("slide-background-path", background_path_);
        update();
    }

protected:
    void paintEvent(QPaintEvent*) override {
        const auto& p = ThemeManager::instance().palette();
        QPainter painter{this};
        painter.setRenderHint(QPainter::Antialiasing, true);
        const QRect bounds = rect();

        if (mode_ == domain::ProgramMode::black) {
            painter.fillRect(bounds, p.preview_black_bg);
            painter.setPen(p.preview_black_text);
            QFont font = painter.font();
            font.setPixelSize(11);
            font.setBold(true);
            font.setLetterSpacing(QFont::AbsoluteSpacing, 2.0);
            painter.setFont(font);
            painter.drawText(bounds, Qt::AlignCenter, QStringLiteral("BLACKOUT"));
            painter.setPen(QPen{p.danger_border, 1.5});
            painter.drawRect(bounds.adjusted(1, 1, -1, -1));
            return;
        }

        const bool has_preview_content =
            !text_.trimmed().isEmpty() || !background_path_.trimmed().isEmpty();
        if (has_preview_content) {
            renderer_.paint(painter, bounds);
        } else {
            painter.fillRect(bounds, p.preview_well_bg);
            painter.setPen(p.preview_well_border);
            painter.drawRect(bounds.adjusted(6, 6, -7, -7));
            painter.setPen(p.preview_well_text);
            QFont font = painter.font();
            font.setPixelSize(10);
            font.setBold(true);
            font.setLetterSpacing(QFont::AbsoluteSpacing, 1.8);
            painter.setFont(font);
            painter.drawText(bounds, Qt::AlignCenter, QStringLiteral("OUTPUT CLEAR"));
        }

        if (mode_ == domain::ProgramMode::cue) {
            painter.setPen(QPen{p.accent, 1.5});
            painter.drawRect(bounds.adjusted(1, 1, -1, -1));
            const QRect badge{8, 8, 42, 16};
            painter.setPen(Qt::NoPen);
            QColor live_badge_bg = p.accent;
            live_badge_bg.setAlpha(220);
            painter.setBrush(live_badge_bg);
            painter.drawRoundedRect(badge, 3, 3);
            painter.setPen(p.text_on_accent);
            QFont badge_font = painter.font();
            badge_font.setPixelSize(9);
            badge_font.setBold(true);
            painter.setFont(badge_font);
            painter.drawText(badge, Qt::AlignCenter, QStringLiteral("LIVE"));
        } else {
            painter.setPen(QPen{p.border_subtle, 1.0});
            painter.drawRect(bounds.adjusted(0, 0, -1, -1));
        }
    }

    domain::ProgramMode mode_{domain::ProgramMode::clear};
    QString text_;
    QString background_path_;
    domain::FitMode background_fit_{domain::FitMode::cover};
    media::SlideRenderer renderer_;
};

[[maybe_unused]] QString mode_text(domain::ProgramMode mode) {
    switch (mode) {
    case domain::ProgramMode::cue:
        return QStringLiteral("LIVE");
    case domain::ProgramMode::clear:
        return QStringLiteral("CLEAR");
    case domain::ProgramMode::black:
        return QStringLiteral("BLACK");
    }
    return QStringLiteral("UNKNOWN");
}

[[maybe_unused]] QString fit_mode_text(domain::FitMode fit) {
    switch (fit) {
    case domain::FitMode::contain:
        return QStringLiteral("Contain");
    case domain::FitMode::cover:
        return QStringLiteral("Cover");
    case domain::FitMode::stretch:
        return QStringLiteral("Stretch");
    }
    return QStringLiteral("Cover");
}

}  // namespace
}  // namespace shekinah::ui
