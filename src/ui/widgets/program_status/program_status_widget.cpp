#include "shekinah/ui/program_status_widget.hpp"

#include <QFrame>
#include <QImageReader>
#include <QLabel>
#include <QHBoxLayout>
#include <QPainter>
#include <QPushButton>
#include <QVBoxLayout>

#include <algorithm>

namespace shekinah::ui {
namespace {

class ProgramSlidePreview final : public QFrame {
public:
    explicit ProgramSlidePreview(QWidget* parent) : QFrame(parent) {
        setAttribute(Qt::WA_OpaquePaintEvent);
        setMinimumHeight(112);
    }

    void set_content(QString text, QString background_path, domain::FitMode background_fit) {
        text_ = std::move(text);
        background_path_ = std::move(background_path);
        background_fit_ = background_fit;
        background_image_ = {};
        background_image_size_ = {};
        setProperty("slide-text", text_);
        setProperty("slide-background-path", background_path_);
        update();
    }

protected:
    void paintEvent(QPaintEvent*) override {
        QPainter painter{this};
        painter.fillRect(rect(), QColor{QStringLiteral("#182337")});
        ensure_background_image();
        if (!background_image_.isNull()) {
            if (background_fit_ == domain::FitMode::stretch) {
                painter.drawImage(rect(), background_image_);
            } else {
                QSize draw_size = background_image_.size();
                draw_size.scale(
                    rect().size(),
                    background_fit_ == domain::FitMode::contain
                        ? Qt::KeepAspectRatio
                        : Qt::KeepAspectRatioByExpanding);
                const QRect draw_rect{
                    rect().center() - QPoint{draw_size.width() / 2, draw_size.height() / 2},
                    draw_size};
                painter.drawImage(draw_rect, background_image_);
            }
        }

        if (text_.trimmed().isEmpty()) {
            return;
        }

        QFont text_font = painter.font();
        text_font.setPixelSize(std::clamp(height() / 8, 10, 24));
        text_font.setBold(true);
        painter.setFont(text_font);
        const auto text_rect = rect().adjusted(12, 12, -12, -12);
        painter.setPen(QColor{QStringLiteral("#080808")});
        painter.drawText(text_rect.translated(1, 1), Qt::AlignCenter | Qt::TextWordWrap, text_);
        painter.setPen(Qt::white);
        painter.drawText(text_rect, Qt::AlignCenter | Qt::TextWordWrap, text_);
    }

private:
    void ensure_background_image() {
        if (background_path_.isEmpty() || background_image_size_ == size()) {
            return;
        }
        background_image_size_ = size();
        background_image_ = {};
        QImageReader reader{background_path_};
        reader.setAutoTransform(true);
        const auto source_size = reader.size();
        if (source_size.isValid() && background_fit_ != domain::FitMode::stretch) {
            auto decoded_size = source_size;
            decoded_size.scale(
                size(),
                background_fit_ == domain::FitMode::contain
                    ? Qt::KeepAspectRatio
                    : Qt::KeepAspectRatioByExpanding);
            reader.setScaledSize(decoded_size);
        } else {
            reader.setScaledSize(size());
        }
        background_image_ = reader.read();
    }

    QString text_;
    QString background_path_;
    domain::FitMode background_fit_{domain::FitMode::cover};
    QImage background_image_;
    QSize background_image_size_;
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

}  // namespace

ProgramStatusWidget::ProgramStatusWidget(QWidget* parent) : QWidget(parent) {
    setObjectName(QStringLiteral("program-status"));
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(8, 8, 8, 8);

    auto* heading = new QLabel{QStringLiteral("PROGRAM STATUS"), this};
    heading->setObjectName(QStringLiteral("program-status-title"));
    heading->setStyleSheet(QStringLiteral("font-size: 12px; font-weight: 600; letter-spacing: 1px;"));
    layout->addWidget(heading);

    auto* preview = new ProgramSlidePreview{this};
    preview->setObjectName(QStringLiteral("program-preview"));
    slide_preview_ = preview;
    layout->addWidget(preview);

    mode_label_ = new QLabel{QStringLiteral("CLEAR"), this};
    mode_label_->setObjectName(QStringLiteral("program-mode"));
    mode_label_->setStyleSheet(QStringLiteral("color: #22e58a; font-size: 11px; font-weight: 600;"));
    mode_label_->setProperty("role", QStringLiteral("status"));
    mode_label_->setAlignment(Qt::AlignCenter);
    cue_label_ = new QLabel{QStringLiteral("No cue live"), this};
    cue_label_->setObjectName(QStringLiteral("program-cue"));
    cue_label_->setStyleSheet(QStringLiteral("color: #22e58a; font-size: 11px;"));
    cue_label_->setAlignment(Qt::AlignCenter);
    layout->addWidget(mode_label_);
    layout->addWidget(cue_label_);

    auto* output_target = new QLabel{QStringLiteral("Output: Monitor 2"), this};
    output_target->setObjectName(QStringLiteral("output-target"));
    auto* audience_mode = new QLabel{QStringLiteral("Audience full-screen"), this};
    audience_mode->setObjectName(QStringLiteral("audience-mode"));
    layout->addWidget(output_target);
    layout->addWidget(audience_mode);

    global_controls_ = new QWidget{this};
    global_controls_->setObjectName(QStringLiteral("global-controls"));
    auto* controls = new QHBoxLayout(global_controls_);
    controls->setContentsMargins(0, 4, 0, 0);
    const auto add_button = [&](const QString& label, const QString& object_name, auto handler) {
        auto* button = new QPushButton{label, global_controls_};
        button->setObjectName(object_name);
        QObject::connect(button, &QPushButton::clicked, this, handler);
        controls->addWidget(button);
    };
    add_button(QStringLiteral("Previous"), QStringLiteral("previous"), [this] {
        if (previous_handler_) {
            previous_handler_();
        }
    });
    add_button(QStringLiteral("Next"), QStringLiteral("next"), [this] {
        if (next_handler_) {
            next_handler_();
        }
    });
    add_button(QStringLiteral("Clear"), QStringLiteral("clear"), [this] {
        if (clear_handler_) {
            clear_handler_();
        }
    });
    add_button(QStringLiteral("Black"), QStringLiteral("black"), [this] {
        if (black_handler_) {
            black_handler_();
        }
    });
    layout->addWidget(global_controls_);
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
    mode_label_->setText(mode_text(program.mode));
    cue_label_->setText(program.live_cue.has_value()
                            ? QString::fromStdString(program.live_cue->cue.value)
                            : QStringLiteral("No cue live"));
    if (!program.live_cue.has_value()) {
        set_slide_preview({});
    }
}

bool ProgramStatusWidget::global_controls_below_status() const noexcept {
    return global_controls_->geometry().top() > cue_label_->geometry().bottom();
}

}  // namespace shekinah::ui
