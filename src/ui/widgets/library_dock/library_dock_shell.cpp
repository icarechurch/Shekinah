#include "shekinah/ui/library_dock_widget.hpp"

#include "shekinah/ui/support/flow_layout.hpp"

#include <QVBoxLayout>

namespace shekinah::ui {

void LibraryDockWidget::setup_toolbar(QWidget* container) {
    auto* toolbar = new QWidget{container};
    toolbar->setObjectName(QStringLiteral("library-toolbar"));
    QSizePolicy toolbar_policy{QSizePolicy::Preferred, QSizePolicy::Preferred};
    toolbar_policy.setHeightForWidth(true);
    toolbar->setSizePolicy(toolbar_policy);
    auto* toolbar_layout = new FlowLayout{toolbar, 0, 6, 4};
    toolbar_layout->setContentsMargins(8, 4, 8, 4);

    setup_toolbar_navigation(toolbar, toolbar_layout);
    setup_toolbar_actions(toolbar, toolbar_layout);
    static_cast<QVBoxLayout*>(container->layout())->addWidget(toolbar);
}

}  // namespace shekinah::ui
