#include "shekinah/ui/program_status_widget.hpp"

#include "shekinah/media/slide_renderer.hpp"
#include "shekinah/ui/cue_grid_widget.hpp"
#include "shekinah/ui/support/elided_label.hpp"

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

QIcon make_glyph_icon(IconGlyph glyph, const QColor& color, int size = 18) {
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
        QPainter painter{this};
        painter.setRenderHint(QPainter::Antialiasing, true);
        const QRect bounds = rect();

        if (mode_ == domain::ProgramMode::black) {
            painter.fillRect(bounds, QColor{QStringLiteral("#040506")});
            painter.setPen(QColor{QStringLiteral("#3f434d")});
            QFont font = painter.font();
            font.setPixelSize(11);
            font.setBold(true);
            font.setLetterSpacing(QFont::AbsoluteSpacing, 2.0);
            painter.setFont(font);
            painter.drawText(bounds, Qt::AlignCenter, QStringLiteral("BLACKOUT"));
            painter.setPen(QPen{QColor{QStringLiteral("#d9383a")}, 1.5});
            painter.drawRect(bounds.adjusted(1, 1, -1, -1));
            return;
        }

        const bool has_preview_content =
            !text_.trimmed().isEmpty() || !background_path_.trimmed().isEmpty();
        if (has_preview_content) {
            renderer_.paint(painter, bounds);
        } else {
            painter.fillRect(bounds, QColor{QStringLiteral("#08090c")});
            painter.setPen(QColor{QStringLiteral("#23262e")});
            painter.drawRect(bounds.adjusted(6, 6, -7, -7));
            painter.setPen(QColor{QStringLiteral("#424652")});
            QFont font = painter.font();
            font.setPixelSize(10);
            font.setBold(true);
            font.setLetterSpacing(QFont::AbsoluteSpacing, 1.8);
            painter.setFont(font);
            painter.drawText(bounds, Qt::AlignCenter, QStringLiteral("OUTPUT CLEAR"));
        }

        if (mode_ == domain::ProgramMode::cue) {
            painter.setPen(QPen{QColor{QStringLiteral("#0a84ff")}, 1.5});
            painter.drawRect(bounds.adjusted(1, 1, -1, -1));
            const QRect badge{8, 8, 42, 16};
            painter.setPen(Qt::NoPen);
            painter.setBrush(QColor{10, 132, 255, 220});
            painter.drawRoundedRect(badge, 3, 3);
            painter.setPen(Qt::white);
            QFont badge_font = painter.font();
            badge_font.setPixelSize(9);
            badge_font.setBold(true);
            painter.setFont(badge_font);
            painter.drawText(badge, Qt::AlignCenter, QStringLiteral("LIVE"));
        } else {
            painter.setPen(QPen{QColor{QStringLiteral("#262830")}, 1.0});
            painter.drawRect(bounds.adjusted(0, 0, -1, -1));
        }
    }

    domain::ProgramMode mode_{domain::ProgramMode::clear};
    QString text_;
    QString background_path_;
    domain::FitMode background_fit_{domain::FitMode::cover};
    media::SlideRenderer renderer_;
};

QString mode_text(domain::ProgramMode mode) {
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

QString fit_mode_text(domain::FitMode fit) {
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

QWidget* make_section_header(const QString& title,
                             QWidget* parent,
                             QWidget* trailing_widget = nullptr,
                             QLabel** title_out = nullptr) {
    auto* bar = new QWidget{parent};
    bar->setObjectName(QStringLiteral("drawer-section-header"));
    bar->setFixedHeight(24);
    bar->setStyleSheet(QStringLiteral(
        "background: #2b2d33; border-top: 1px solid #16171b; border-bottom: 1px solid #16171b;"));
    auto* row = new QHBoxLayout(bar);
    row->setContentsMargins(8, 0, 6, 0);
    row->setSpacing(4);
    auto* label = new QLabel{title, bar};
    label->setStyleSheet(QStringLiteral(
        "color: #b8bcc4; font-size: 10px; font-weight: 700; letter-spacing: 0.8px; border: none;"));
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

ProgramStatusWidget::ProgramStatusWidget(QWidget* parent) : QWidget(parent) {
    setObjectName(QStringLiteral("program-status"));
    setStyleSheet(QStringLiteral(
        "#program-status {"
        "  background-color: #1e1f24;"
        "  border-left: 1px solid #111215;"
        "  color: #e6e8ec;"
        "}"
        "#program-status QPushButton#rail-btn,"
        "#program-status QPushButton[rail=\"true\"] {"
        "  background-color: #1b1c20;"
        "  border: none;"
        "  border-bottom: 1px solid #262830;"
        "  border-radius: 0px;"
        "  padding: 0px;"
        "}"
        "#program-status QPushButton[rail=\"true\"]:hover {"
        "  background-color: #282a31;"
        "}"
        "#program-status QPushButton[rail=\"true\"]:checked {"
        "  background-color: #0a84ff;"
        "}"
        "#program-status QPushButton#global-btn,"
        "#program-status QWidget#global-controls QPushButton {"
        "  background-color: #292b32;"
        "  border: 1px solid #383b44;"
        "  border-radius: 4px;"
        "  color: #e8eaed;"
        "  font-size: 10px;"
        "  font-weight: 600;"
        "  padding: 4px 4px;"
        "  min-height: 24px;"
        "}"
        "#program-status QWidget#global-controls QPushButton:hover {"
        "  background-color: #343741;"
        "  border-color: #4b4f5b;"
        "}"
        "#program-status QWidget#global-controls QPushButton:pressed {"
        "  background-color: #1f2127;"
        "}"
        "#program-status QListWidget {"
        "  background-color: #1a1b1f;"
        "  border: none;"
        "  color: #dfe2e7;"
        "  font-size: 11px;"
        "  outline: 0;"
        "}"
        "#program-status QListWidget::item {"
        "  padding: 5px 8px;"
        "  border-bottom: 1px solid #23252b;"
        "}"
        "#program-status QListWidget::item:selected {"
        "  background-color: #27446b;"
        "  color: #ffffff;"
        "}"
        "#program-status QListWidget::item:hover {"
        "  background-color: #24262c;"
        "}"
        "#program-status QLineEdit {"
        "  background-color: #16171a;"
        "  border: 1px solid #30333b;"
        "  border-radius: 4px;"
        "  color: #e8eaed;"
        "  font-size: 11px;"
        "  padding: 3px 8px;"
        "}"
        "#program-status QLineEdit:focus {"
        "  border-color: #0a84ff;"
        "}"));

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // 1. Compact Header Bar ("PROGRAM STATUS" + mode pill)
    auto* header_bar = new QWidget{this};
    header_bar->setObjectName(QStringLiteral("program-header-bar"));
    header_bar->setFixedHeight(26);
    header_bar->setStyleSheet(QStringLiteral(
        "background-color: #191a1e; border-bottom: 1px solid #111215;"));
    auto* header_layout = new QHBoxLayout(header_bar);
    header_layout->setContentsMargins(10, 0, 8, 0);
    header_layout->setSpacing(6);

    auto* heading = new QLabel{QStringLiteral("PROGRAM STATUS"), header_bar};
    heading->setObjectName(QStringLiteral("program-status-title"));
    heading->setStyleSheet(QStringLiteral(
        "color: #8e929b; font-size: 10px; font-weight: 700; letter-spacing: 1.1px; border: none;"));
    header_layout->addWidget(heading);
    header_layout->addStretch(1);

    mode_label_ = new QLabel{QStringLiteral("CLEAR"), header_bar};
    mode_label_->setObjectName(QStringLiteral("program-mode"));
    mode_label_->setProperty("role", QStringLiteral("status"));
    mode_label_->setAlignment(Qt::AlignCenter);
    header_layout->addWidget(mode_label_);
    layout->addWidget(header_bar);

    // 2. Monitor + Circular Clear-All + Vertical Quick-Action Rail
    auto* monitor_row = new QWidget{this};
    monitor_row->setObjectName(QStringLiteral("monitor-row"));
    monitor_row->setStyleSheet(QStringLiteral("background-color: #121316;"));
    auto* monitor_layout = new QHBoxLayout(monitor_row);
    monitor_layout->setContentsMargins(6, 6, 0, 6);
    monitor_layout->setSpacing(4);

    auto* preview = new ProgramSlidePreview{monitor_row};
    preview->setObjectName(QStringLiteral("program-preview"));
    slide_preview_ = preview;
    monitor_layout->addWidget(preview, 1);

    // Floating-style circular Clear All button between preview and rail (ProPresenter hallmark)
    auto* clear_gutter = new QWidget{monitor_row};
    clear_gutter->setFixedWidth(22);
    auto* gutter_layout = new QVBoxLayout(clear_gutter);
    gutter_layout->setContentsMargins(0, 0, 0, 0);
    gutter_layout->addStretch(1);
    quick_clear_button_ = new QPushButton{clear_gutter};
    quick_clear_button_->setObjectName(QStringLiteral("quick-clear-all"));
    quick_clear_button_->setFixedSize(20, 20);
    quick_clear_button_->setCursor(Qt::PointingHandCursor);
    quick_clear_button_->setToolTip(QStringLiteral("Clear All Output (Esc)"));
    quick_clear_button_->setIcon(make_glyph_icon(IconGlyph::clear_all, QColor{QStringLiteral("#141518")}, 12));
    quick_clear_button_->setIconSize(QSize{12, 12});
    quick_clear_button_->setStyleSheet(QStringLiteral(
        "QPushButton#quick-clear-all {"
        "  background-color: #e6e8ec;"
        "  border: 1px solid #141518;"
        "  border-radius: 10px;"
        "}"
        "QPushButton#quick-clear-all:hover { background-color: #ffffff; }"
        "QPushButton#quick-clear-all:pressed { background-color: #bfc4cd; }"));
    QObject::connect(quick_clear_button_, &QPushButton::clicked, this, [this] {
        if (clear_handler_) {
            clear_handler_();
        }
    });
    gutter_layout->addWidget(quick_clear_button_, 0, Qt::AlignCenter);
    gutter_layout->addStretch(1);
    monitor_layout->addWidget(clear_gutter);

    // Right vertical quick-action rail
    auto* rail = new QWidget{monitor_row};
    rail->setObjectName(QStringLiteral("program-action-rail"));
    rail->setFixedWidth(34);
    rail->setStyleSheet(QStringLiteral(
        "background-color: #1b1c20; border-left: 1px solid #111215;"));
    auto* rail_layout = new QVBoxLayout(rail);
    rail_layout->setContentsMargins(0, 0, 0, 0);
    rail_layout->setSpacing(0);

    const auto add_rail_button = [&](const QString& name,
                                     const QString& tooltip,
                                     IconGlyph glyph,
                                     bool checkable,
                                     auto handler) -> QPushButton* {
        auto* btn = new QPushButton{rail};
        btn->setObjectName(name);
        btn->setProperty("rail", true);
        btn->setCheckable(checkable);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setToolTip(tooltip);
        btn->setFixedSize(34, 28);
        btn->setIcon(make_glyph_icon(glyph, QColor{QStringLiteral("#c5c9d2")}, 15));
        btn->setIconSize(QSize{15, 15});
        QObject::connect(btn, &QPushButton::clicked, this, handler);
        rail_layout->addWidget(btn);
        return btn;
    };

    rail_clear_button_ = add_rail_button(
        QStringLiteral("rail-clear-cue"), QStringLiteral("Clear Live Slide (Esc)"),
        IconGlyph::slide, false, [this] {
            if (clear_handler_) {
                clear_handler_();
            }
        });
    rail_background_button_ = add_rail_button(
        QStringLiteral("rail-background"), QStringLiteral("Set Background on Live Cue"),
        IconGlyph::media_image, false, [this] {
            if (choose_background_handler_) {
                choose_background_handler_();
            }
        });
    rail_black_button_ = add_rail_button(
        QStringLiteral("rail-blackout"), QStringLiteral("Blackout Audience Output (B)"),
        IconGlyph::blackout, true, [this] {
            if (black_handler_) {
                black_handler_();
            }
        });
    rail_audience_button_ = add_rail_button(
        QStringLiteral("rail-audience"), QStringLiteral("Show Audience Output Window"),
        IconGlyph::display, true, [this] {
            if (output_visible_) {
                if (hide_output_handler_) {
                    hide_output_handler_();
                }
            } else if (show_movable_handler_) {
                show_movable_handler_();
            }
        });
    rail_inspector_button_ = add_rail_button(
        QStringLiteral("rail-inspector"), QStringLiteral("Open Slide Inspector"),
        IconGlyph::sliders, false, [this] {
            if (inspect_live_handler_) {
                inspect_live_handler_();
            }
        });
    rail_layout->addStretch(1);
    monitor_layout->addWidget(rail);
    layout->addWidget(monitor_row);

    // 3. Screen Selector Bar ("Output: Monitor 2 v  |  Audience full-screen")
    auto* screen_bar = new QWidget{this};
    screen_bar->setObjectName(QStringLiteral("screen-selector-bar"));
    screen_bar->setFixedHeight(26);
    screen_bar->setStyleSheet(QStringLiteral(
        "#screen-selector-bar {"
        "  background-color: #24252a;"
        "  border-top: 1px solid #131417;"
        "  border-bottom: 1px solid #131417;"
        "}"));
    auto* screen_layout = new QHBoxLayout(screen_bar);
    screen_layout->setContentsMargins(8, 0, 8, 0);
    screen_layout->setSpacing(4);

    output_target_label_ = new ElidedLabel{QStringLiteral("Output: Monitor 2"), screen_bar};
    output_target_label_->setObjectName(QStringLiteral("output-target"));
    output_target_label_->setStyleSheet(QStringLiteral(
        "color: #d7dae0; font-size: 11px; font-weight: 600; border: none;"));
    screen_layout->addWidget(output_target_label_);

    screen_menu_button_ = new QPushButton{screen_bar};
    screen_menu_button_->setObjectName(QStringLiteral("screen-menu-button"));
    screen_menu_button_->setFixedSize(18, 18);
    screen_menu_button_->setCursor(Qt::PointingHandCursor);
    screen_menu_button_->setToolTip(QStringLiteral("Select Audience Output Display"));
    screen_menu_button_->setIcon(
        make_glyph_icon(IconGlyph::chevron_down, QColor{QStringLiteral("#9ca1ab")}, 12));
    screen_menu_button_->setIconSize(QSize{12, 12});
    screen_menu_button_->setStyleSheet(QStringLiteral(
        "QPushButton { background: transparent; border: none; border-radius: 3px; }"
        "QPushButton:hover { background: #32353d; }"));
    QObject::connect(screen_menu_button_, &QPushButton::clicked, this, [this] { open_screen_menu(); });
    screen_layout->addWidget(screen_menu_button_);
    screen_layout->addStretch(1);

    audience_mode_label_ = new ElidedLabel{QStringLiteral("Audience full-screen"), screen_bar};
    audience_mode_label_->setObjectName(QStringLiteral("audience-mode"));
    audience_mode_label_->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    audience_mode_label_->setStyleSheet(QStringLiteral(
        "color: #8e929b; font-size: 10px; border: none;"));
    screen_layout->addWidget(audience_mode_label_);
    layout->addWidget(screen_bar);

    // 4. Layer Status Strip + Live Cue Identifier (cue_label_ MUST stay above global_controls_)
    auto* status_strip = new QWidget{this};
    status_strip->setObjectName(QStringLiteral("program-status-strip"));
    status_strip->setStyleSheet(QStringLiteral("background-color: #1c1d22;"));
    auto* status_layout = new QVBoxLayout(status_strip);
    status_layout->setContentsMargins(8, 6, 8, 6);
    status_layout->setSpacing(4);

    auto* indicator_row = new QHBoxLayout;
    indicator_row->setSpacing(10);
    indicator_row->addStretch(1);
    const auto make_indicator = [&](const QString& name, const QString& tip, IconGlyph glyph) {
        auto* btn = new QPushButton{status_strip};
        btn->setObjectName(name);
        btn->setToolTip(tip);
        btn->setFixedSize(26, 20);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setIcon(make_glyph_icon(glyph, QColor{QStringLiteral("#6d727d")}, 14));
        btn->setIconSize(QSize{14, 14});
        btn->setStyleSheet(QStringLiteral(
            "QPushButton { background: transparent; border: none; border-radius: 3px; }"
            "QPushButton:hover { background: #292b33; }"));
        indicator_row->addWidget(btn);
        return btn;
    };
    status_screen_indicator_ = make_indicator(
        QStringLiteral("indicator-screen"), QStringLiteral("Slide Layer Status"), IconGlyph::slide);
    status_media_indicator_ = make_indicator(
        QStringLiteral("indicator-media"), QStringLiteral("Background Media Status"), IconGlyph::media_image);
    status_audience_indicator_ = make_indicator(
        QStringLiteral("indicator-audience"), QStringLiteral("Toggle Audience Output Window"), IconGlyph::display);
    QObject::connect(status_screen_indicator_, &QPushButton::clicked, this, [this] {
        set_active_tab(0);
    });
    QObject::connect(status_media_indicator_, &QPushButton::clicked, this, [this] {
        if (choose_background_handler_) {
            choose_background_handler_();
        }
    });
    QObject::connect(status_audience_indicator_, &QPushButton::clicked, this, [this] {
        set_active_tab(1);
    });
    indicator_row->addStretch(1);
    status_layout->addLayout(indicator_row);

    cue_label_ = new ElidedLabel{QStringLiteral("No cue live"), status_strip};
    cue_label_->setObjectName(QStringLiteral("program-cue"));
    cue_label_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    cue_label_->setAlignment(Qt::AlignCenter);
    cue_label_->setStyleSheet(QStringLiteral("color: #8e929b; font-size: 11px; font-weight: 500;"));
    status_layout->addWidget(cue_label_);
    layout->addWidget(status_strip);

    // 5. Slide Progress Track + Transport Controls (global-controls)
    slide_progress_ = new QProgressBar{this};
    slide_progress_->setObjectName(QStringLiteral("slide-progress"));
    slide_progress_->setTextVisible(false);
    slide_progress_->setFixedHeight(3);
    slide_progress_->setRange(0, 100);
    slide_progress_->setValue(0);
    slide_progress_->setStyleSheet(QStringLiteral(
        "QProgressBar { background-color: #131417; border: none; }"
        "QProgressBar::chunk { background-color: #0a84ff; }"));
    layout->addWidget(slide_progress_);

    global_controls_ = new QWidget{this};
    global_controls_->setObjectName(QStringLiteral("global-controls"));
    global_controls_->setStyleSheet(QStringLiteral(
        "#global-controls { background-color: #1c1d22; border-bottom: 1px solid #111215; }"));
    auto* controls = new QHBoxLayout(global_controls_);
    controls->setContentsMargins(6, 6, 6, 8);
    controls->setSpacing(4);

    const auto add_control = [&](const QString& label,
                                 const QString& object_name,
                                 IconGlyph glyph,
                                 auto handler) -> QPushButton* {
        auto* button = new QPushButton{label, global_controls_};
        button->setObjectName(object_name);
        button->setCursor(Qt::PointingHandCursor);
        button->setIcon(make_glyph_icon(glyph, QColor{QStringLiteral("#d2d6de")}, 11));
        button->setIconSize(QSize{11, 11});
        QObject::connect(button, &QPushButton::clicked, this, handler);
        controls->addWidget(button, 1);
        return button;
    };

    previous_button_ = add_control(
        QStringLiteral("Previous"), QStringLiteral("previous"), IconGlyph::previous, [this] {
            if (previous_handler_) {
                previous_handler_();
            }
        });
    next_button_ = add_control(
        QStringLiteral("Next"), QStringLiteral("next"), IconGlyph::next, [this] {
            if (next_handler_) {
                next_handler_();
            }
        });
    clear_button_ = add_control(
        QStringLiteral("Clear"), QStringLiteral("clear"), IconGlyph::clear_layer, [this] {
            if (clear_handler_) {
                clear_handler_();
            }
        });
    black_button_ = add_control(
        QStringLiteral("Black"), QStringLiteral("black"), IconGlyph::blackout, [this] {
            if (black_handler_) {
                black_handler_();
            }
        });
    layout->addWidget(global_controls_);

    // 6. ProPresenter Segmented Icon Tab Bar
    auto* tab_bar = new QWidget{this};
    tab_bar->setObjectName(QStringLiteral("drawer-tab-bar"));
    tab_bar->setFixedHeight(28);
    tab_bar->setStyleSheet(QStringLiteral(
        "#drawer-tab-bar { background-color: #292b31; border-bottom: 1px solid #121316; }"));
    auto* tab_layout = new QHBoxLayout(tab_bar);
    tab_layout->setContentsMargins(0, 0, 0, 0);
    tab_layout->setSpacing(1);

    const auto add_tab = [&](const QString& name, const QString& tip, IconGlyph glyph, int idx) {
        auto* btn = new QPushButton{tab_bar};
        btn->setObjectName(name);
        btn->setToolTip(tip);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setCheckable(true);
        btn->setFixedHeight(27);
        btn->setIcon(make_glyph_icon(glyph, QColor{QStringLiteral("#e8eaed")}, 15));
        btn->setIconSize(QSize{15, 15});
        QObject::connect(btn, &QPushButton::clicked, this, [this, idx] { set_active_tab(idx); });
        tab_layout->addWidget(btn, 1);
        return btn;
    };
    tab_live_button_ = add_tab(
        QStringLiteral("tab-live"), QStringLiteral("Live & Next Slide"), IconGlyph::layers, 0);
    tab_screens_button_ = add_tab(
        QStringLiteral("tab-screens"), QStringLiteral("Screens & Diagnostics"), IconGlyph::display, 1);
    tab_cues_button_ = add_tab(
        QStringLiteral("tab-cues"), QStringLiteral("Active Item Cues"), IconGlyph::list, 2);
    layout->addWidget(tab_bar);

    // 7. Stacked Drawer Pages
    drawer_stack_ = new QStackedWidget{this};
    drawer_stack_->setObjectName(QStringLiteral("drawer-stack"));
    drawer_stack_->setMinimumHeight(60);
    drawer_stack_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Ignored);

    // --- Page 0: Live & Next (scrollable when window height is constrained) ---
    auto* live_scroll = new QScrollArea{drawer_stack_};
    live_scroll->setObjectName(QStringLiteral("drawer-page-live-scroll"));
    live_scroll->setMinimumHeight(48);
    live_scroll->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Ignored);
    live_scroll->setFrameShape(QFrame::NoFrame);
    live_scroll->setWidgetResizable(true);
    live_scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    live_scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    live_scroll->setStyleSheet(QStringLiteral("background-color: #1d1e23;"));

    auto* live_page = new QWidget{live_scroll};
    live_page->setObjectName(QStringLiteral("drawer-page-live"));
    live_page->setStyleSheet(QStringLiteral("background-color: #1d1e23;"));
    auto* live_page_layout = new QVBoxLayout(live_page);
    live_page_layout->setContentsMargins(0, 0, 0, 0);
    live_page_layout->setSpacing(0);

    live_edit_button_ = new QPushButton{QStringLiteral("Inspect"), live_page};
    live_edit_button_->setObjectName(QStringLiteral("live-inspect-button"));
    live_edit_button_->setCursor(Qt::PointingHandCursor);
    live_edit_button_->setStyleSheet(QStringLiteral(
        "QPushButton { background: transparent; border: none; color: #8ec5ff; font-size: 10px; font-weight: 600; padding: 2px 4px; }"
        "QPushButton:hover { color: #ffffff; }"));
    QObject::connect(live_edit_button_, &QPushButton::clicked, this, [this] {
        if (inspect_live_handler_) {
            inspect_live_handler_();
        }
    });
    live_page_layout->addWidget(
        make_section_header(QStringLiteral("LIVE SLIDE"), live_page, live_edit_button_));

    auto* live_card = new QWidget{live_page};
    live_card->setStyleSheet(QStringLiteral("background-color: #191a1f;"));
    auto* live_card_layout = new QVBoxLayout(live_card);
    live_card_layout->setContentsMargins(10, 8, 10, 8);
    live_card_layout->setSpacing(6);

    auto* meta_row = new QHBoxLayout;
    meta_row->setSpacing(6);
    live_section_badge_ = new QLabel{QStringLiteral("IDLE"), live_card};
    live_section_badge_->setObjectName(QStringLiteral("live-section-badge"));
    live_section_badge_->setStyleSheet(QStringLiteral(
        "background-color: #2b2d34; color: #b6bac4; border-radius: 3px; padding: 2px 6px; font-size: 10px; font-weight: 700;"));
    meta_row->addWidget(live_section_badge_);

    live_item_title_label_ = new ElidedLabel{QStringLiteral("Ready for service"), live_card};
    live_item_title_label_->setObjectName(QStringLiteral("live-item-title"));
    live_item_title_label_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    live_item_title_label_->setStyleSheet(QStringLiteral(
        "color: #e6e8ec; font-size: 11px; font-weight: 600;"));
    meta_row->addWidget(live_item_title_label_, 1);

    live_slide_counter_label_ = new QLabel{QStringLiteral("--"), live_card};
    live_slide_counter_label_->setObjectName(QStringLiteral("live-slide-counter"));
    live_slide_counter_label_->setStyleSheet(QStringLiteral(
        "color: #8e929b; font-size: 10px; font-weight: 600;"));
    meta_row->addWidget(live_slide_counter_label_);
    live_card_layout->addLayout(meta_row);

    live_lyrics_label_ = new QLabel{
        QStringLiteral("Click any cue in the grid to project it live to the Audience Output."),
        live_card};
    live_lyrics_label_->setObjectName(QStringLiteral("live-lyrics-preview"));
    live_lyrics_label_->setMinimumWidth(0);
    live_lyrics_label_->setWordWrap(true);
    live_lyrics_label_->setStyleSheet(QStringLiteral(
        "color: #9ea3ae; font-size: 11px; background-color: #141519; border: 1px solid #25272f; border-radius: 4px; padding: 6px 8px;"));
    live_card_layout->addWidget(live_lyrics_label_);

    auto* bg_row = new QHBoxLayout;
    bg_row->setSpacing(6);
    live_asset_label_ = new ElidedLabel{QStringLiteral("Background: None"), live_card};
    live_asset_label_->setObjectName(QStringLiteral("live-asset-label"));
    live_asset_label_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    live_asset_label_->setStyleSheet(QStringLiteral("color: #8e929b; font-size: 10px;"));
    bg_row->addWidget(live_asset_label_, 1);

    live_bg_button_ = new QPushButton{QStringLiteral("Set Bg"), live_card};
    live_bg_button_->setObjectName(QStringLiteral("live-set-bg-button"));
    live_bg_button_->setCursor(Qt::PointingHandCursor);
    live_bg_button_->setStyleSheet(QStringLiteral(
        "QPushButton { background: #262830; border: 1px solid #363944; border-radius: 3px; color: #cfd3dc; font-size: 10px; padding: 2px 7px; }"
        "QPushButton:hover { background: #31343e; }"));
    QObject::connect(live_bg_button_, &QPushButton::clicked, this, [this] {
        if (choose_background_handler_) {
            choose_background_handler_();
        }
    });
    bg_row->addWidget(live_bg_button_);
    live_card_layout->addLayout(bg_row);
    live_page_layout->addWidget(live_card);

    auto* next_btn = new QPushButton{live_page};
    next_btn->setObjectName(QStringLiteral("next-up-header-button"));
    next_btn->setCursor(Qt::PointingHandCursor);
    next_btn->setToolTip(QStringLiteral("Advance to Next Cue (PgDn)"));
    next_btn->setIcon(make_glyph_icon(IconGlyph::arrow_right, QColor{QStringLiteral("#b8bcc4")}, 13));
    next_btn->setIconSize(QSize{13, 13});
    next_btn->setStyleSheet(QStringLiteral(
        "QPushButton { background: transparent; border: none; padding: 2px; }"
        "QPushButton:hover { background: #3a3d46; border-radius: 3px; }"));
    QObject::connect(next_btn, &QPushButton::clicked, this, [this] {
        if (next_handler_) {
            next_handler_();
        }
    });
    live_page_layout->addWidget(make_section_header(QStringLiteral("NEXT UP"), live_page, next_btn));

    next_up_card_ = new QWidget{live_page};
    next_up_card_->setStyleSheet(QStringLiteral("background-color: #1b1c21;"));
    auto* next_layout = new QVBoxLayout(next_up_card_);
    next_layout->setContentsMargins(10, 8, 10, 8);
    next_layout->setSpacing(6);

    auto* next_meta = new QHBoxLayout;
    next_section_badge_ = new QLabel{QStringLiteral("STANDBY"), next_up_card_};
    next_section_badge_->setObjectName(QStringLiteral("next-section-badge"));
    next_section_badge_->setStyleSheet(QStringLiteral(
        "background-color: #272930; color: #9ea3ae; border-radius: 3px; padding: 2px 6px; font-size: 10px; font-weight: 700;"));
    next_meta->addWidget(next_section_badge_);
    next_meta->addStretch(1);
    next_layout->addLayout(next_meta);

    next_summary_label_ = new QLabel{
        QStringLiteral("Select a Song or Sermon in the Service Order to cue slides."),
        next_up_card_};
    next_summary_label_->setObjectName(QStringLiteral("next-summary-label"));
    next_summary_label_->setMinimumWidth(0);
    next_summary_label_->setWordWrap(true);
    next_summary_label_->setStyleSheet(QStringLiteral("color: #9ea3ae; font-size: 11px;"));
    next_layout->addWidget(next_summary_label_);

    next_up_action_button_ = new QPushButton{QStringLiteral("Show Audience Window"), next_up_card_};
    next_up_action_button_->setObjectName(QStringLiteral("next-up-action-button"));
    next_up_action_button_->setCursor(Qt::PointingHandCursor);
    next_up_action_button_->setStyleSheet(QStringLiteral(
        "QPushButton { background-color: #282a32; border: 1px solid #383b46; border-radius: 5px; color: #e2e5ea; font-size: 11px; font-weight: 600; padding: 6px 10px; }"
        "QPushButton:hover { background-color: #32353f; border-color: #0a84ff; }"));
    QObject::connect(next_up_action_button_, &QPushButton::clicked, this, [this] {
        if (current_program_.mode == domain::ProgramMode::cue && !live_details_.next_summary.isEmpty()) {
            if (next_handler_) {
                next_handler_();
            }
        } else if (show_movable_handler_) {
            show_movable_handler_();
        }
    });
    next_layout->addWidget(next_up_action_button_);
    next_layout->addStretch(1);
    live_page_layout->addWidget(next_up_card_, 1);
    live_scroll->setWidget(live_page);
    drawer_stack_->addWidget(live_scroll);

    // --- Page 1: Screens & Diagnostics (scrollable when height is constrained) ---
    auto* screens_scroll = new QScrollArea{drawer_stack_};
    screens_scroll->setObjectName(QStringLiteral("drawer-page-screens-scroll"));
    screens_scroll->setMinimumHeight(48);
    screens_scroll->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Ignored);
    screens_scroll->setFrameShape(QFrame::NoFrame);
    screens_scroll->setWidgetResizable(true);
    screens_scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    screens_scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    screens_scroll->setStyleSheet(QStringLiteral("background-color: #1d1e23;"));

    auto* screens_page = new QWidget{screens_scroll};
    screens_page->setObjectName(QStringLiteral("drawer-page-screens"));
    screens_page->setStyleSheet(QStringLiteral("background-color: #1d1e23;"));
    auto* screens_layout = new QVBoxLayout(screens_page);
    screens_layout->setContentsMargins(0, 0, 0, 0);
    screens_layout->setSpacing(0);

    auto* refresh_screens_btn = new QPushButton{QStringLiteral("Refresh"), screens_page};
    refresh_screens_btn->setCursor(Qt::PointingHandCursor);
    refresh_screens_btn->setStyleSheet(QStringLiteral(
        "QPushButton { background: transparent; border: none; color: #8ec5ff; font-size: 10px; font-weight: 600; padding: 2px 4px; }"
        "QPushButton:hover { color: #ffffff; }"));
    QObject::connect(refresh_screens_btn, &QPushButton::clicked, this, [this] {
        refresh_screens_list();
    });
    screens_layout->addWidget(
        make_section_header(QStringLiteral("AUDIENCE DISPLAYS"), screens_page, refresh_screens_btn));

    screens_list_ = new QListWidget{screens_page};
    screens_list_->setObjectName(QStringLiteral("screens-list"));
    screens_list_->setMinimumHeight(48);
    screens_list_->setSelectionMode(QAbstractItemView::SingleSelection);
    QObject::connect(screens_list_, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem* item) {
        if (item == nullptr || !show_screen_handler_) {
            return;
        }
        const int idx = item->data(Qt::UserRole).toInt();
        const auto screens = QGuiApplication::screens();
        if (idx >= 0 && idx < screens.size()) {
            show_screen_handler_(screens.at(idx));
        }
    });
    screens_layout->addWidget(screens_list_, 1);

    auto* screen_actions = new QWidget{screens_page};
    screen_actions->setStyleSheet(QStringLiteral("background-color: #1a1b1f;"));
    auto* screen_actions_layout = new QHBoxLayout(screen_actions);
    screen_actions_layout->setContentsMargins(8, 6, 8, 6);
    screen_actions_layout->setSpacing(6);

    const auto add_screen_btn = [&](const QString& label, const QString& name, auto handler) {
        auto* btn = new QPushButton{label, screen_actions};
        btn->setObjectName(name);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setStyleSheet(QStringLiteral(
            "QPushButton { background: #282a32; border: 1px solid #383b44; border-radius: 4px; color: #e2e5ea; font-size: 10px; font-weight: 600; padding: 4px 6px; }"
            "QPushButton:hover { background: #333640; }"));
        QObject::connect(btn, &QPushButton::clicked, this, handler);
        screen_actions_layout->addWidget(btn, 1);
    };
    add_screen_btn(QStringLiteral("Full-Screen"), QStringLiteral("output-fullscreen-btn"), [this] {
        if (!show_screen_handler_) {
            return;
        }
        const auto screens = QGuiApplication::screens();
        const int row = screens_list_->currentRow();
        QScreen* target = (row >= 0 && row < screens.size())
                              ? screens.at(row)
                              : (screens.size() > 1 ? screens.at(1) : QGuiApplication::primaryScreen());
        if (target != nullptr) {
            show_screen_handler_(target);
        }
    });
    add_screen_btn(QStringLiteral("Window"), QStringLiteral("output-window-btn"), [this] {
        if (show_movable_handler_) {
            show_movable_handler_();
        }
    });
    add_screen_btn(QStringLiteral("Hide"), QStringLiteral("output-hide-btn"), [this] {
        if (hide_output_handler_) {
            hide_output_handler_();
        }
    });
    screens_layout->addWidget(screen_actions);

    screens_layout->addWidget(make_section_header(QStringLiteral("RESOURCE GOVERNOR"), screens_page));
    auto* mem_panel = new QWidget{screens_page};
    mem_panel->setStyleSheet(QStringLiteral("background-color: #191a1f;"));
    auto* mem_layout = new QVBoxLayout(mem_panel);
    mem_layout->setContentsMargins(10, 8, 10, 10);
    mem_layout->setSpacing(6);

    memory_status_label_ = new QLabel{
        QStringLiteral("Memory: -- / 270 MB · Live Video Admitted"), mem_panel};
    memory_status_label_->setObjectName(QStringLiteral("memory-status-label"));
    memory_status_label_->setMinimumWidth(0);
    memory_status_label_->setWordWrap(true);
    memory_status_label_->setStyleSheet(QStringLiteral("color: #b8bcc4; font-size: 10px;"));
    mem_layout->addWidget(memory_status_label_);

    memory_bar_ = new QProgressBar{mem_panel};
    memory_bar_->setObjectName(QStringLiteral("memory-usage-bar"));
    memory_bar_->setTextVisible(false);
    memory_bar_->setFixedHeight(6);
    memory_bar_->setRange(0, 300);
    memory_bar_->setValue(0);
    memory_bar_->setStyleSheet(QStringLiteral(
        "QProgressBar { background-color: #121316; border: 1px solid #282b33; border-radius: 3px; }"
        "QProgressBar::chunk { background-color: #22e58a; border-radius: 2px; }"));
    mem_layout->addWidget(memory_bar_);
    screens_layout->addWidget(mem_panel);
    screens_scroll->setWidget(screens_page);
    drawer_stack_->addWidget(screens_scroll);

    // --- Page 2: Active Item Cues + Filter ---
    auto* cues_page = new QWidget{drawer_stack_};
    cues_page->setObjectName(QStringLiteral("drawer-page-cues"));
    cues_page->setStyleSheet(QStringLiteral("background-color: #1d1e23;"));
    auto* cues_layout = new QVBoxLayout(cues_page);
    cues_layout->setContentsMargins(0, 0, 0, 0);
    cues_layout->setSpacing(0);

    cues_layout->addWidget(
        make_section_header(QStringLiteral("0 CUES"), cues_page, nullptr, &cues_count_label_));

    cues_list_ = new QListWidget{cues_page};
    cues_list_->setObjectName(QStringLiteral("quick-cues-list"));
    cues_list_->setMinimumHeight(36);
    cues_list_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Ignored);
    cues_list_->setSelectionMode(QAbstractItemView::SingleSelection);
    QObject::connect(cues_list_, &QListWidget::itemClicked, this, [this](QListWidgetItem* item) {
        if (item == nullptr || !take_cue_handler_) {
            return;
        }
        const auto id = item->data(Qt::UserRole).toString();
        if (!id.isEmpty()) {
            take_cue_handler_(domain::CueId{id.toStdString()});
        }
    });
    cues_layout->addWidget(cues_list_, 1);

    auto* filter_bar = new QWidget{cues_page};
    filter_bar->setObjectName(QStringLiteral("cues-filter-bar"));
    filter_bar->setStyleSheet(QStringLiteral(
        "background-color: #23252a; border-top: 1px solid #131417;"));
    auto* filter_layout = new QHBoxLayout(filter_bar);
    filter_layout->setContentsMargins(8, 5, 8, 5);
    filter_layout->setSpacing(6);

    cues_filter_input_ = new QLineEdit{filter_bar};
    cues_filter_input_->setObjectName(QStringLiteral("quick-cues-filter"));
    cues_filter_input_->setMinimumWidth(60);
    cues_filter_input_->setPlaceholderText(QStringLiteral("Filter cues..."));
    cues_filter_input_->setClearButtonEnabled(true);
    QObject::connect(cues_filter_input_, &QLineEdit::textChanged, this, [this] {
        apply_cue_filter();
    });
    filter_layout->addWidget(cues_filter_input_, 1);

    auto* shortcut_hint = new ElidedLabel{QStringLiteral("PgUp/Dn · Esc · B"), filter_bar};
    shortcut_hint->setStyleSheet(QStringLiteral("color: #777c87; font-size: 10px; border: none;"));
    filter_layout->addWidget(shortcut_hint);
    cues_layout->addWidget(filter_bar);
    drawer_stack_->addWidget(cues_page);

    layout->addWidget(drawer_stack_, 1);

    refresh_screens_list();
    set_active_tab(0);
    update_status_chrome();
}

void ProgramStatusWidget::set_program_title(QString title) {
    const auto trimmed = title.trimmed();
    set_slide_preview(trimmed.isEmpty() ? QStringLiteral("No cue live") : trimmed);
}

void ProgramStatusWidget::set_slide_preview(QString text,
                                            QString background_path,
                                            domain::FitMode background_fit) {
    auto* preview = static_cast<ProgramSlidePreview*>(slide_preview_);
    preview->set_content(std::move(text), std::move(background_path), background_fit);
}

void ProgramStatusWidget::set_program(const domain::ProgramState& program) {
    current_program_ = program;
    auto* preview = static_cast<ProgramSlidePreview*>(slide_preview_);
    preview->set_mode(program.mode);
    mode_label_->setText(mode_text(program.mode));
    cue_label_->setText(program.live_cue.has_value()
                            ? QString::fromStdString(program.live_cue->cue.value)
                            : QStringLiteral("No cue live"));
    if (!program.live_cue.has_value()) {
        set_slide_preview({});
        live_details_ = {};
        set_live_details(live_details_);
    }
    update_status_chrome();
}

void ProgramStatusWidget::set_live_details(LiveSlideDetails details) {
    live_details_ = std::move(details);

    const bool has_live =
        current_program_.mode == domain::ProgramMode::cue && current_program_.live_cue.has_value();

    if (!has_live && live_details_.slide_text.isEmpty() && live_details_.item_title.isEmpty()) {
        live_section_badge_->setText(
            current_program_.mode == domain::ProgramMode::black ? QStringLiteral("BLACK")
                                                                : QStringLiteral("IDLE"));
        live_section_badge_->setStyleSheet(QStringLiteral(
            "background-color: #2b2d34; color: #b6bac4; border-radius: 3px; padding: 2px 6px; font-size: 10px; font-weight: 700;"));
        live_item_title_label_->setText(
            active_item_title_.isEmpty() ? QStringLiteral("Ready for service") : active_item_title_);
        live_slide_counter_label_->setText(QStringLiteral("--"));
        live_lyrics_label_->setText(
            QStringLiteral("Click any cue in the grid to project it live to the Audience Output."));
        live_asset_label_->setText(QStringLiteral("Background: None"));
        slide_progress_->setValue(0);
    } else {
        const QString section_label = live_details_.section.trimmed().isEmpty()
                                          ? QStringLiteral("SLIDE")
                                          : live_details_.section.trimmed().toUpper();
        const QColor section_color = CueGridWidget::footer_color_for_section(live_details_.section);
        live_section_badge_->setText(section_label);
        live_section_badge_->setStyleSheet(
            QStringLiteral("background-color: %1; color: #ffffff; border-radius: 3px; padding: 2px 6px; font-size: 10px; font-weight: 700;")
                .arg(section_color.name()));
        live_item_title_label_->setText(
            live_details_.item_title.isEmpty() ? QStringLiteral("Live Cue") : live_details_.item_title);
        if (live_details_.total_slides > 0 && live_details_.slide_index > 0) {
            live_slide_counter_label_->setText(
                QStringLiteral("%1 / %2").arg(live_details_.slide_index).arg(live_details_.total_slides));
            const int pct = std::clamp(
                static_cast<int>(std::lround(100.0 * static_cast<double>(live_details_.slide_index) /
                                             static_cast<double>(live_details_.total_slides))),
                0,
                100);
            slide_progress_->setValue(pct);
        } else {
            live_slide_counter_label_->setText(QStringLiteral("LIVE"));
            slide_progress_->setValue(100);
        }
        live_lyrics_label_->setText(
            live_details_.slide_text.trimmed().isEmpty()
                ? QStringLiteral("Media / image slide active")
                : live_details_.slide_text.trimmed());
        if (live_details_.has_background) {
            const QString kind = live_details_.is_video ? QStringLiteral("Video")
                                                        : fit_mode_text(live_details_.fit_mode);
            live_asset_label_->setText(
                live_details_.asset_name.isEmpty()
                    ? QStringLiteral("Background: Active (%1)").arg(kind)
                    : QStringLiteral("Bg: %1 · %2").arg(live_details_.asset_name, kind));
        } else {
            live_asset_label_->setText(QStringLiteral("Background: None"));
        }
    }

    if (!live_details_.next_summary.trimmed().isEmpty() || !live_details_.next_section.trimmed().isEmpty()) {
        const QString next_sec = live_details_.next_section.trimmed().isEmpty()
                                     ? QStringLiteral("NEXT")
                                     : live_details_.next_section.trimmed().toUpper();
        const QColor next_color = CueGridWidget::footer_color_for_section(live_details_.next_section);
        next_section_badge_->setText(next_sec);
        next_section_badge_->setStyleSheet(
            QStringLiteral("background-color: %1; color: #ffffff; border-radius: 3px; padding: 2px 6px; font-size: 10px; font-weight: 700;")
                .arg(next_color.name()));
        next_summary_label_->setText(
            live_details_.next_summary.trimmed().isEmpty()
                ? QStringLiteral("Media / Sermon slide")
                : live_details_.next_summary.trimmed());
        next_up_action_button_->setText(QStringLiteral("Go to Next Slide"));
    } else {
        next_section_badge_->setText(has_live ? QStringLiteral("END") : QStringLiteral("STANDBY"));
        next_section_badge_->setStyleSheet(QStringLiteral(
            "background-color: #272930; color: #9ea3ae; border-radius: 3px; padding: 2px 6px; font-size: 10px; font-weight: 700;"));
        next_summary_label_->setText(
            has_live
                ? QStringLiteral("End of current presentation item.")
                : QStringLiteral("Select a Song or Sermon in the Service Order to cue slides."));
        next_up_action_button_->setText(QStringLiteral("Show Audience Window"));
    }
    update_status_chrome();
}

void ProgramStatusWidget::set_item_cues(QString item_title, std::vector<QuickCueItem> cues) {
    active_item_title_ = std::move(item_title);
    item_cues_ = std::move(cues);
    cues_list_->clear();
    for (const auto& cue : item_cues_) {
        const QString num = cue.number > 0 ? QString::number(cue.number) : QStringLiteral("•");
        const QString sec = cue.section.trimmed().isEmpty() ? QStringLiteral("Slide") : cue.section.trimmed();
        QString first_line = cue.summary.section(QLatin1Char('\n'), 0, 0).trimmed();
        if (first_line.isEmpty()) {
            first_line = QStringLiteral("(media slide)");
        }
        auto* item = new QListWidgetItem{QStringLiteral("%1. [%2] %3").arg(num, sec, first_line)};
        item->setData(Qt::UserRole, QString::fromStdString(cue.id.value));
        item->setToolTip(cue.summary);
        cues_list_->addItem(item);
    }
    cues_count_label_->setText(
        QStringLiteral("%1 CUE%2")
            .arg(static_cast<qulonglong>(item_cues_.size()))
            .arg(item_cues_.size() == 1 ? QString() : QStringLiteral("S")));
    apply_cue_filter();
}

void ProgramStatusWidget::set_output_status(QString target_name, QString mode_label, bool output_visible) {
    if (!target_name.trimmed().isEmpty()) {
        output_target_label_->setText(target_name);
    }
    if (!mode_label.trimmed().isEmpty()) {
        audience_mode_label_->setText(mode_label);
    }
    output_visible_ = output_visible;
    update_status_chrome();
}

void ProgramStatusWidget::set_memory_status(std::size_t working_set_bytes, bool live_media_admitted) {
    const double mb = static_cast<double>(working_set_bytes) / (1024.0 * 1024.0);
    const int mb_int = static_cast<int>(std::lround(mb));
    memory_bar_->setValue(std::clamp(mb_int, 0, 300));
    const QString chunk_color = !live_media_admitted
                                    ? QStringLiteral("#ff5a5f")
                                    : (mb >= 220.0 ? QStringLiteral("#f5a623") : QStringLiteral("#22e58a"));
    memory_bar_->setStyleSheet(
        QStringLiteral(
            "QProgressBar { background-color: #121316; border: 1px solid #282b33; border-radius: 3px; }"
            "QProgressBar::chunk { background-color: %1; border-radius: 2px; }")
            .arg(chunk_color));
    memory_status_label_->setText(
        QStringLiteral("Memory: %1 MB / 270 MB · %2")
            .arg(mb, 0, 'f', 1)
            .arg(live_media_admitted ? QStringLiteral("Live Video Admitted")
                                     : QStringLiteral("Live Video Throttled")));
}

bool ProgramStatusWidget::global_controls_below_status() const noexcept {
    return global_controls_->geometry().top() > cue_label_->geometry().bottom();
}

void ProgramStatusWidget::set_active_tab(int index) {
    const int clamped = std::clamp(index, 0, drawer_stack_->count() - 1);
    drawer_stack_->setCurrentIndex(clamped);

    const std::pair<QPushButton*, IconGlyph> tabs[] = {
        {tab_live_button_, IconGlyph::layers},
        {tab_screens_button_, IconGlyph::display},
        {tab_cues_button_, IconGlyph::list},
    };
    for (int i = 0; i < 3; ++i) {
        const bool active = (i == clamped);
        auto* btn = tabs[i].first;
        btn->setChecked(active);
        btn->setStyleSheet(active
                               ? QStringLiteral("background-color: #0a84ff; border: none;")
                               : QStringLiteral("background-color: #2f3138; border: none;"));
        btn->setIcon(make_glyph_icon(
            tabs[i].second,
            active ? QColor{Qt::white} : QColor{QStringLiteral("#c5c9d2")},
            15));
    }
    if (clamped == 1) {
        refresh_screens_list();
    }
}

void ProgramStatusWidget::update_status_chrome() {
    switch (current_program_.mode) {
    case domain::ProgramMode::cue:
        mode_label_->setStyleSheet(QStringLiteral(
            "background-color: rgba(34, 229, 138, 0.16); color: #22e58a; border: 1px solid rgba(34, 229, 138, 0.45); border-radius: 3px; padding: 1px 6px; font-size: 10px; font-weight: 700;"));
        cue_label_->setStyleSheet(QStringLiteral("color: #f1f3f5; font-size: 11px; font-weight: 600;"));
        break;
    case domain::ProgramMode::black:
        mode_label_->setStyleSheet(QStringLiteral(
            "background-color: rgba(217, 56, 58, 0.22); color: #ff6b6d; border: 1px solid rgba(217, 56, 58, 0.55); border-radius: 3px; padding: 1px 6px; font-size: 10px; font-weight: 700;"));
        cue_label_->setStyleSheet(QStringLiteral("color: #ff8e8f; font-size: 11px; font-weight: 600;"));
        break;
    case domain::ProgramMode::clear:
        mode_label_->setStyleSheet(QStringLiteral(
            "background-color: #262830; color: #22e58a; border: 1px solid #353844; border-radius: 3px; padding: 1px 6px; font-size: 10px; font-weight: 700;"));
        cue_label_->setStyleSheet(QStringLiteral("color: #22e58a; font-size: 11px; font-weight: 500;"));
        break;
    }

    const bool is_live = current_program_.mode == domain::ProgramMode::cue;
    const bool is_black = current_program_.mode == domain::ProgramMode::black;
    rail_black_button_->setChecked(is_black);
    rail_audience_button_->setChecked(output_visible_);

    status_screen_indicator_->setIcon(make_glyph_icon(
        IconGlyph::slide,
        is_live ? QColor{QStringLiteral("#0a84ff")}
                : is_black ? QColor{QStringLiteral("#ff5a5f")}
                           : QColor{QStringLiteral("#6d727d")},
        14));
    status_media_indicator_->setIcon(make_glyph_icon(
        IconGlyph::media_image,
        (is_live && live_details_.has_background) ? QColor{QStringLiteral("#0a84ff")}
                                                 : QColor{QStringLiteral("#6d727d")},
        14));
    status_audience_indicator_->setIcon(make_glyph_icon(
        IconGlyph::display,
        output_visible_ ? QColor{QStringLiteral("#0a84ff")} : QColor{QStringLiteral("#6d727d")},
        14));

    black_button_->setStyleSheet(
        is_black
            ? QStringLiteral("background-color: #7d2326; border-color: #d9383a; color: #ffffff;")
            : QString{});
}

void ProgramStatusWidget::refresh_screens_list() {
    screens_list_->clear();
    const auto screens = QGuiApplication::screens();
    for (int i = 0; i < screens.size(); ++i) {
        const auto* screen = screens.at(i);
        if (screen == nullptr) {
            continue;
        }
        const QRect geo = screen->geometry();
        const QString primary_tag =
            (screen == QGuiApplication::primaryScreen()) ? QStringLiteral(" (Primary)") : QString{};
        const QString label = QStringLiteral("Monitor %1 — %2 (%3×%4)%5")
                                  .arg(i + 1)
                                  .arg(screen->name())
                                  .arg(geo.width())
                                  .arg(geo.height())
                                  .arg(primary_tag);
        auto* item = new QListWidgetItem{label};
        item->setData(Qt::UserRole, i);
        screens_list_->addItem(item);
    }
    if (screens_list_->count() > 0) {
        screens_list_->setCurrentRow(screens_list_->count() > 1 ? 1 : 0);
    }
}

void ProgramStatusWidget::apply_cue_filter() {
    const QString query = cues_filter_input_->text().trimmed();
    for (int i = 0; i < cues_list_->count(); ++i) {
        auto* item = cues_list_->item(i);
        const bool matches = query.isEmpty() ||
                             item->text().contains(query, Qt::CaseInsensitive) ||
                             item->toolTip().contains(query, Qt::CaseInsensitive);
        item->setHidden(!matches);
    }
}

void ProgramStatusWidget::open_screen_menu() {
    QMenu menu{this};
    const auto screens = QGuiApplication::screens();
    for (int i = 0; i < screens.size(); ++i) {
        auto* screen = screens.at(i);
        if (screen == nullptr) {
            continue;
        }
        const QRect geo = screen->geometry();
        const QString label = QStringLiteral("Monitor %1 (%2×%3) — Full-screen")
                                  .arg(i + 1)
                                  .arg(geo.width())
                                  .arg(geo.height());
        auto* action = menu.addAction(label);
        QObject::connect(action, &QAction::triggered, this, [this, screen] {
            if (show_screen_handler_) {
                show_screen_handler_(screen);
            }
        });
    }
    menu.addSeparator();
    auto* movable_action = menu.addAction(QStringLiteral("Audience Window (Movable)"));
    QObject::connect(movable_action, &QAction::triggered, this, [this] {
        if (show_movable_handler_) {
            show_movable_handler_();
        }
    });
    auto* hide_action = menu.addAction(QStringLiteral("Hide Audience Output"));
    QObject::connect(hide_action, &QAction::triggered, this, [this] {
        if (hide_output_handler_) {
            hide_output_handler_();
        }
    });
    menu.exec(screen_menu_button_->mapToGlobal(QPoint{0, screen_menu_button_->height()}));
}

}  // namespace shekinah::ui
