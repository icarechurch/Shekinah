#include "shekinah/ui/program_status_widget.hpp"

#include <QHBoxLayout>
#include <QPushButton>
#include <QVBoxLayout>

namespace shekinah::ui {

void ProgramStatusWidget::build_global_actions(QVBoxLayout* layout) {
    global_controls_ = new QWidget{this};
    global_controls_->setObjectName(QStringLiteral("global-controls"));
    auto* controls = new QHBoxLayout(global_controls_);
    controls->setContentsMargins(6, 6, 6, 8);
    controls->setSpacing(4);

    const auto add_control = [&](const QString& label,
                                 const QString& object_name,
                                 auto handler) -> QPushButton* {
        auto* button = new QPushButton{label, global_controls_};
        button->setObjectName(object_name);
        button->setCursor(Qt::PointingHandCursor);
        button->setIconSize(QSize{11, 11});
        QObject::connect(button, &QPushButton::clicked, this, handler);
        controls->addWidget(button, 1);
        return button;
    };

    previous_button_ = add_control(
        QStringLiteral("Previous"), QStringLiteral("previous"), [this] {
            if (previous_handler_) {
                previous_handler_();
            }
        });
    next_button_ = add_control(
        QStringLiteral("Next"), QStringLiteral("next"), [this] {
            if (next_handler_) {
                next_handler_();
            }
        });
    clear_button_ = add_control(
        QStringLiteral("Clear"), QStringLiteral("clear"), [this] {
            if (clear_handler_) {
                clear_handler_();
            }
        });
    black_button_ = add_control(
        QStringLiteral("Black"), QStringLiteral("black"), [this] {
            if (black_handler_) {
                black_handler_();
            }
        });
    layout->addWidget(global_controls_);
}

}  // namespace shekinah::ui
