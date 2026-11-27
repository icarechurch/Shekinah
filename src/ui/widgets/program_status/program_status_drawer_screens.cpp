#include "shekinah/ui/program_status_widget.hpp"

#include "program_status_sections.hpp"

#include <QAbstractItemView>
#include <QFrame>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QListWidget>
#include <QPushButton>
#include <QScrollArea>
#include <QSizePolicy>
#include <QStackedWidget>
#include <QVBoxLayout>

namespace shekinah::ui {

void ProgramStatusWidget::build_screens_drawer_page() {
    auto* screens_scroll = new QScrollArea{drawer_stack_};
    screens_scroll->setObjectName(QStringLiteral("drawer-page-screens-scroll"));
    screens_scroll->setMinimumHeight(48);
    screens_scroll->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Ignored);
    screens_scroll->setFrameShape(QFrame::NoFrame);
    screens_scroll->setWidgetResizable(true);
    screens_scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    screens_scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    auto* screens_page = new QWidget{screens_scroll};
    screens_page->setObjectName(QStringLiteral("drawer-page-screens"));
    auto* screens_layout = new QVBoxLayout(screens_page);
    screens_layout->setContentsMargins(0, 0, 0, 0);
    screens_layout->setSpacing(0);

    auto* refresh_screens_btn = new QPushButton{QStringLiteral("Refresh"), screens_page};
    refresh_screens_btn->setProperty("linkAction", true);
    refresh_screens_btn->setCursor(Qt::PointingHandCursor);
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
    screen_actions->setObjectName(QStringLiteral("screen-actions-bar"));
    auto* screen_actions_layout = new QHBoxLayout(screen_actions);
    screen_actions_layout->setContentsMargins(8, 6, 8, 6);
    screen_actions_layout->setSpacing(6);

    const auto add_screen_btn = [&](const QString& label, const QString& name, auto handler) {
        auto* btn = new QPushButton{label, screen_actions};
        btn->setObjectName(name);
        btn->setProperty("screenAction", true);
        btn->setCursor(Qt::PointingHandCursor);
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

    build_resource_governor(screens_page, screens_layout);
    screens_scroll->setWidget(screens_page);
    drawer_stack_->addWidget(screens_scroll);
}

}  // namespace shekinah::ui
