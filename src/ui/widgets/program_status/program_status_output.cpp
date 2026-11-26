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
        const QString label = QStringLiteral("Monitor %1 â€” %2 (%3Ã—%4)%5")
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
        const QString label = QStringLiteral("Monitor %1 (%2Ã—%3) â€” Full-screen")
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
