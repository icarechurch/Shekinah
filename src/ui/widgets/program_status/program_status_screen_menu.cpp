#include "shekinah/ui/program_status_widget.hpp"

#include <QAction>
#include <QGuiApplication>
#include <QMenu>
#include <QPushButton>
#include <QScreen>

namespace shekinah::ui {

void ProgramStatusWidget::open_screen_menu() {
    QMenu menu{this};
    const auto screens = QGuiApplication::screens();
    for (int i = 0; i < screens.size(); ++i) {
        auto* screen = screens.at(i);
        if (screen == nullptr) {
            continue;
        }
        const QRect geo = screen->geometry();
        const QString label = QStringLiteral("Monitor %1 (%2Ãƒâ€”%3) Ã¢â‚¬â€ Full-screen")
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
