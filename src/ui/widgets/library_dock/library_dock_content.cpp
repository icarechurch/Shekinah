#include "shekinah/ui/library_dock_widget.hpp"

#include "library_components.hpp"
#include <QVBoxLayout>
#include <QStackedWidget>
#include <QListWidget>

namespace shekinah::ui {
using namespace library_internal;

void LibraryDockWidget::setup_content(QWidget* container) {
    stack_ = new QStackedWidget{container};
    stack_->setObjectName(QStringLiteral("library-stack"));
    setup_asset_list();
    setup_content_lists();
    stack_->addWidget(assets_);
    stack_->addWidget(songs_);
    stack_->addWidget(sermons_);
    static_cast<QVBoxLayout*>(container->layout())->addWidget(stack_, 1);
    setup_content_empty_overlay();
}

}  // namespace shekinah::ui
