#include "shekinah/ui/main_window.hpp"

#include "shekinah/ui/support/theme.hpp"

#include <QAction>
#include <QActionGroup>
#include <QMenu>
#include <QMenuBar>

namespace shekinah::ui {

void ShekinahWindow::configure_view_menu() {
    auto* view_menu = menuBar()->addMenu(QStringLiteral("View"));
    view_menu->setObjectName(QStringLiteral("view-menu"));
    auto* theme_menu = view_menu->addMenu(QStringLiteral("Theme"));
    theme_menu->setObjectName(QStringLiteral("theme-menu"));
    theme_actions_ = new QActionGroup{this};
    theme_actions_->setExclusive(true);
    for (const auto& info : ThemeManager::available_themes()) {
        auto* action = theme_menu->addAction(info.label);
        action->setObjectName(QStringLiteral("theme-%1").arg(info.key));
        action->setCheckable(true);
        action->setData(static_cast<int>(info.id));
        action->setStatusTip(info.description);
        theme_actions_->addAction(action);
        QObject::connect(action, &QAction::triggered, this, [this, id = info.id] {
            set_theme(id);
        });
        if (info.id == ThemeId::system) {
            theme_menu->addSeparator();
        }
    }
}

}  // namespace shekinah::ui
