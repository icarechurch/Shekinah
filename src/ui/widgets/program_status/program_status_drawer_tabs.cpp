#include "shekinah/ui/program_status_widget.hpp"

#include <QHBoxLayout>
#include <QPushButton>
#include <QVBoxLayout>

namespace shekinah::ui {

void ProgramStatusWidget::build_drawer_tabs(QVBoxLayout* layout) {
    auto* tab_bar = new QWidget{this};
    tab_bar->setObjectName(QStringLiteral("drawer-tab-bar"));
    tab_bar->setFixedHeight(28);
    auto* tab_layout = new QHBoxLayout(tab_bar);
    tab_layout->setContentsMargins(0, 0, 0, 0);
    tab_layout->setSpacing(1);

    const auto add_tab = [&](const QString& name, const QString& tip, int idx) {
        auto* btn = new QPushButton{tab_bar};
        btn->setObjectName(name);
        btn->setToolTip(tip);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setCheckable(true);
        btn->setFixedHeight(27);
        btn->setIconSize(QSize{15, 15});
        QObject::connect(btn, &QPushButton::clicked, this, [this, idx] { set_active_tab(idx); });
        tab_layout->addWidget(btn, 1);
        return btn;
    };
    tab_live_button_ = add_tab(QStringLiteral("tab-live"), QStringLiteral("Live & Next Slide"), 0);
    tab_screens_button_ = add_tab(
        QStringLiteral("tab-screens"), QStringLiteral("Screens & Diagnostics"), 1);
    tab_cues_button_ = add_tab(QStringLiteral("tab-cues"), QStringLiteral("Active Item Cues"), 2);
    layout->addWidget(tab_bar);
}

}  // namespace shekinah::ui
