#include "shekinah/ui/program_status_widget.hpp"

#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

namespace shekinah::ui {
namespace {

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

    mode_label_ = new QLabel{QStringLiteral("CLEAR"), this};
    mode_label_->setObjectName(QStringLiteral("program-mode"));
    mode_label_->setProperty("role", QStringLiteral("status"));
    cue_label_ = new QLabel{QStringLiteral("No cue live"), this};
    cue_label_->setObjectName(QStringLiteral("program-cue"));
    layout->addWidget(mode_label_);
    layout->addWidget(cue_label_);

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

void ProgramStatusWidget::set_program(const domain::ProgramState& program) {
    mode_label_->setText(mode_text(program.mode));
    cue_label_->setText(program.live_cue.has_value()
                            ? QString::fromStdString(program.live_cue->cue.value)
                            : QStringLiteral("No cue live"));
}

bool ProgramStatusWidget::global_controls_below_status() const noexcept {
    return global_controls_->geometry().top() > cue_label_->geometry().bottom();
}

}  // namespace shekinah::ui
