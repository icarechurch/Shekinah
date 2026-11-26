#pragma once

#include "shekinah/ui/program_status_widget.hpp"

#include "shekinah/media/slide_renderer.hpp"
#include "shekinah/ui/cue_grid_widget.hpp"
#include "shekinah/ui/support/elided_label.hpp"
#include "shekinah/ui/support/theme.hpp"

#include <QAbstractItemView>
#include <QFrame>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMenu>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QProgressBar>
#include <QPushButton>
#include <QScreen>
#include <QScrollArea>
#include <QStackedWidget>
#include <QVBoxLayout>

#include <algorithm>
#include <cmath>

namespace shekinah::ui {
namespace {

enum class IconGlyph {
    clear_all,
    slide,
    media_image,
    blackout,
    display,
    sliders,
    chevron_down,
    previous,
    next,
    clear_layer,
    layers,
    list,
    plus,
    arrow_right,
};

[[maybe_unused]] QIcon make_glyph_icon(IconGlyph glyph, const QColor& color, int size = 18) {
    QPixmap pixmap{size * 2, size * 2};
    pixmap.fill(Qt::transparent);
    pixmap.setDevicePixelRatio(2.0);
    QPainter p{&pixmap};
    p.setRenderHint(QPainter::Antialiasing, true);
    QPen pen{color, 1.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin};
    p.setPen(pen);
    const qreal s = static_cast<qreal>(size);

    switch (glyph) {
    case IconGlyph::clear_all: {
        p.drawLine(QPointF{s * 0.30, s * 0.30}, QPointF{s * 0.70, s * 0.70});
        p.drawLine(QPointF{s * 0.70, s * 0.30}, QPointF{s * 0.30, s * 0.70});
        break;
    }
    case IconGlyph::slide: {
        p.drawRoundedRect(QRectF{s * 0.14, s * 0.22, s * 0.72, s * 0.50}, 2.0, 2.0);
        p.drawLine(QPointF{s * 0.34, s * 0.82}, QPointF{s * 0.66, s * 0.82});
        p.drawLine(QPointF{s * 0.50, s * 0.72}, QPointF{s * 0.50, s * 0.82});
        break;
    }
    case IconGlyph::media_image: {
        p.drawRoundedRect(QRectF{s * 0.14, s * 0.18, s * 0.72, s * 0.64}, 2.0, 2.0);
        p.drawEllipse(QPointF{s * 0.34, s * 0.38}, s * 0.07, s * 0.07);
        p.drawLine(QPointF{s * 0.22, s * 0.72}, QPointF{s * 0.44, s * 0.50});
        p.drawLine(QPointF{s * 0.44, s * 0.50}, QPointF{s * 0.58, s * 0.64});
        p.drawLine(QPointF{s * 0.58, s * 0.64}, QPointF{s * 0.68, s * 0.54});
        p.drawLine(QPointF{s * 0.68, s * 0.54}, QPointF{s * 0.78, s * 0.72});
        break;
    }
    case IconGlyph::blackout: {
        p.drawRoundedRect(QRectF{s * 0.16, s * 0.22, s * 0.68, s * 0.52}, 2.0, 2.0);
        p.drawLine(QPointF{s * 0.22, s * 0.78}, QPointF{s * 0.78, s * 0.18});
        break;
    }
    case IconGlyph::display: {
        p.drawRoundedRect(QRectF{s * 0.14, s * 0.18, s * 0.72, s * 0.50}, 2.0, 2.0);
        p.drawLine(QPointF{s * 0.30, s * 0.80}, QPointF{s * 0.70, s * 0.80});
        p.drawLine(QPointF{s * 0.50, s * 0.68}, QPointF{s * 0.50, s * 0.80});
        p.drawRect(QRectF{s * 0.26, s * 0.30, s * 0.48, s * 0.26});
        break;
    }
    case IconGlyph::sliders: {
        p.drawLine(QPointF{s * 0.18, s * 0.30}, QPointF{s * 0.82, s * 0.30});
        p.drawLine(QPointF{s * 0.18, s * 0.52}, QPointF{s * 0.82, s * 0.52});
        p.drawLine(QPointF{s * 0.18, s * 0.74}, QPointF{s * 0.82, s * 0.74});
        p.setBrush(color);
        p.drawEllipse(QPointF{s * 0.36, s * 0.30}, 1.8, 1.8);
        p.drawEllipse(QPointF{s * 0.64, s * 0.52}, 1.8, 1.8);
        p.drawEllipse(QPointF{s * 0.44, s * 0.74}, 1.8, 1.8);
        break;
    }
    case IconGlyph::chevron_down: {
        p.drawLine(QPointF{s * 0.26, s * 0.38}, QPointF{s * 0.50, s * 0.64});
        p.drawLine(QPointF{s * 0.50, s * 0.64}, QPointF{s * 0.74, s * 0.38});
        break;
    }
    case IconGlyph::previous: {
        p.drawLine(QPointF{s * 0.28, s * 0.26}, QPointF{s * 0.28, s * 0.74});
        QPainterPath tri;
        tri.moveTo(s * 0.72, s * 0.26);
        tri.lineTo(s * 0.38, s * 0.50);
        tri.lineTo(s * 0.72, s * 0.74);
        tri.closeSubpath();
        p.setBrush(color);
        p.drawPath(tri);
        break;
    }
    case IconGlyph::next: {
        p.drawLine(QPointF{s * 0.72, s * 0.26}, QPointF{s * 0.72, s * 0.74});
        QPainterPath tri;
        tri.moveTo(s * 0.28, s * 0.26);
        tri.lineTo(s * 0.62, s * 0.50);
        tri.lineTo(s * 0.28, s * 0.74);
        tri.closeSubpath();
        p.setBrush(color);
        p.drawPath(tri);
        break;
    }
    case IconGlyph::clear_layer: {
        p.drawEllipse(QPointF{s * 0.50, s * 0.50}, s * 0.32, s * 0.32);
        p.drawLine(QPointF{s * 0.37, s * 0.37}, QPointF{s * 0.63, s * 0.63});
        p.drawLine(QPointF{s * 0.63, s * 0.37}, QPointF{s * 0.37, s * 0.63});
        break;
    }
    case IconGlyph::layers: {
        QPainterPath top;
        top.moveTo(s * 0.50, s * 0.18);
        top.lineTo(s * 0.82, s * 0.36);
        top.lineTo(s * 0.50, s * 0.54);
        top.lineTo(s * 0.18, s * 0.36);
        top.closeSubpath();
        p.drawPath(top);
        p.drawLine(QPointF{s * 0.18, s * 0.54}, QPointF{s * 0.50, s * 0.70});
        p.drawLine(QPointF{s * 0.50, s * 0.70}, QPointF{s * 0.82, s * 0.54});
        p.drawLine(QPointF{s * 0.18, s * 0.70}, QPointF{s * 0.50, s * 0.86});
        p.drawLine(QPointF{s * 0.50, s * 0.86}, QPointF{s * 0.82, s * 0.70});
        break;
    }
    case IconGlyph::list: {
        p.drawLine(QPointF{s * 0.34, s * 0.30}, QPointF{s * 0.82, s * 0.30});
        p.drawLine(QPointF{s * 0.34, s * 0.50}, QPointF{s * 0.82, s * 0.50});
        p.drawLine(QPointF{s * 0.34, s * 0.70}, QPointF{s * 0.82, s * 0.70});
        p.setBrush(color);
        p.drawEllipse(QPointF{s * 0.20, s * 0.30}, 1.3, 1.3);
        p.drawEllipse(QPointF{s * 0.20, s * 0.50}, 1.3, 1.3);
        p.drawEllipse(QPointF{s * 0.20, s * 0.70}, 1.3, 1.3);
        break;
    }
    case IconGlyph::plus: {
        p.drawLine(QPointF{s * 0.50, s * 0.22}, QPointF{s * 0.50, s * 0.78});
        p.drawLine(QPointF{s * 0.22, s * 0.50}, QPointF{s * 0.78, s * 0.50});
        break;
    }
    case IconGlyph::arrow_right: {
        p.drawLine(QPointF{s * 0.22, s * 0.50}, QPointF{s * 0.76, s * 0.50});
        p.drawLine(QPointF{s * 0.54, s * 0.28}, QPointF{s * 0.76, s * 0.50});
        p.drawLine(QPointF{s * 0.54, s * 0.72}, QPointF{s * 0.76, s * 0.50});
        break;
    }
    }
    return QIcon{pixmap};
}

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

[[maybe_unused]] QWidget* make_section_header(const QString& title,
                                              QWidget* parent,
                                              QWidget* trailing_widget = nullptr,
                                              QLabel** title_out = nullptr) {
    auto* bar = new QWidget{parent};
    bar->setObjectName(QStringLiteral("drawer-section-header"));
    bar->setProperty("sectionHeader", true);
    bar->setFixedHeight(24);
    auto* row = new QHBoxLayout(bar);
    row->setContentsMargins(8, 0, 6, 0);
    row->setSpacing(4);
    auto* label = new QLabel{title, bar};
    label->setProperty("sectionTitle", true);
    row->addWidget(label);
    row->addStretch(1);
    if (trailing_widget != nullptr) {
        row->addWidget(trailing_widget);
    }
    if (title_out != nullptr) {
        *title_out = label;
    }
    return bar;
}

}  // namespace
}  // namespace shekinah::ui
