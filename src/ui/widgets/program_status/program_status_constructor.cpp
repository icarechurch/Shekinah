#include "shekinah/ui/program_status_widget.hpp"

#include "shekinah/ui/support/theme.hpp"

#include <QVBoxLayout>

namespace shekinah::ui {

ProgramStatusWidget::ProgramStatusWidget(QWidget* parent) : QWidget(parent) {
    setObjectName(QStringLiteral("program-status"));

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    build_header(layout);
    build_monitor(layout);
    build_screen_selector(layout);
    build_status_strip(layout);
    build_global_controls(layout);
    build_drawer_tabs(layout);
    build_drawer_pages(layout);

    refresh_screens_list();
    set_active_tab(0);
    apply_theme();
    QObject::connect(&ThemeManager::instance(), &ThemeManager::theme_changed, this,
                     [this](ThemeId) { apply_theme(); });
}

}  // namespace shekinah::ui
