#include <QApplication>

#include "shekinah/ui/main_window.hpp"

int main(int argc, char* argv[]) {
    QApplication application(argc, argv);
    shekinah::ui::ShekinahWindow window;
    window.show();
    return application.exec();
}
