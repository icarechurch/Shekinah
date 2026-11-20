#include "shekinah/ui/main_window.hpp"

#include "shekinah/ui/library_dock_widget.hpp"
#include "shekinah/ui/operator_page.hpp"

namespace shekinah::ui {

ShekinahWindow::ShekinahWindow(QWidget* parent) : QMainWindow(parent) {
    setObjectName(QStringLiteral("shekinah-window"));
    setWindowTitle(QStringLiteral("Shekinah"));
    resize(1280, 720);

    operator_page_ = new OperatorPage{this};
    setCentralWidget(operator_page_);
    library_dock_ = new LibraryDockWidget{this};
    addDockWidget(Qt::BottomDockWidgetArea, library_dock_);
}

}  // namespace shekinah::ui
