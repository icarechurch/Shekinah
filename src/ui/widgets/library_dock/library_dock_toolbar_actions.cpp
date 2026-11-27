#include "shekinah/ui/library_dock_widget.hpp"

#include "library_components.hpp"

#include "shekinah/ui/support/flow_layout.hpp"

#include <QHBoxLayout>
#include <QPushButton>
#include <QSize>

namespace shekinah::ui {
using namespace library_internal;

void LibraryDockWidget::setup_toolbar_actions(QWidget* toolbar, FlowLayout* toolbar_layout) {
    setup_asset_toolbar_actions(toolbar, toolbar_layout);
    setup_song_toolbar_actions(toolbar, toolbar_layout);
    setup_sermon_toolbar_actions(toolbar, toolbar_layout);
}

std::pair<QWidget*, QHBoxLayout*> LibraryDockWidget::create_toolbar_action_group(
    QWidget* toolbar, FlowLayout* toolbar_layout) {
    auto* group = new QWidget{toolbar};
    group->setStyleSheet(QStringLiteral("background: transparent; border: none;"));
    auto* layout = new QHBoxLayout{group};
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(4);
    toolbar_layout->addWidget(group);
    return {group, layout};
}

QPushButton* LibraryDockWidget::add_toolbar_action(QWidget* group_parent,
                                                    QHBoxLayout* group_layout,
                                                    const QString& label,
                                                    const QString& object_name,
                                                    const QString& tooltip,
                                                    LibraryGlyph glyph,
                                                    const QColor& icon_color,
                                                    std::function<void()> handler) {
    auto* button = new QPushButton{label, group_parent};
    button->setObjectName(object_name);
    button->setProperty("libAction", true);
    button->setProperty("primaryAction", false);
    button->setFixedHeight(26);
    button->setCursor(Qt::PointingHandCursor);
    button->setToolTip(tooltip);
    button->setIcon(make_library_icon(glyph, icon_color, 12));
    button->setIconSize(QSize{12, 12});
    QObject::connect(button, &QPushButton::clicked, this, [handler = std::move(handler)] {
        handler();
    });
    group_layout->addWidget(button);
    return button;
}

}  // namespace shekinah::ui
