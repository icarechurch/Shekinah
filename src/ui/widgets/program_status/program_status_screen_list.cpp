#include "shekinah/ui/program_status_widget.hpp"

#include <QGuiApplication>
#include <QListWidget>
#include <QListWidgetItem>
#include <QScreen>

namespace shekinah::ui {

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
        const QString label = QStringLiteral("Monitor %1 Ã¢â‚¬â€ %2 (%3Ãƒâ€”%4)%5")
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

}  // namespace shekinah::ui
