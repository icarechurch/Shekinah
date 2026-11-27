#include "program_status_preview_paint.hpp"

#include "shekinah/ui/support/theme.hpp"

#include <QFont>
#include <QPainter>
#include <QPen>

namespace shekinah::ui {

void paint_program_slide_preview(QPainter& painter,
                                 const QRect& bounds,
                                 domain::ProgramMode mode,
                                 const QString& text,
                                 const QString& background_path,
                                 media::SlideRenderer& renderer) {
    const auto& p = ThemeManager::instance().palette();

    if (mode == domain::ProgramMode::black) {
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

    const bool has_preview_content = !text.trimmed().isEmpty() || !background_path.trimmed().isEmpty();
    if (has_preview_content) {
        renderer.paint(painter, bounds);
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

    if (mode == domain::ProgramMode::cue) {
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

}  // namespace shekinah::ui
