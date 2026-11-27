#include "shekinah/ui/library_dock_widget.hpp"

#include <QKeySequence>
#include <QLineEdit>
#include <QShortcut>

namespace shekinah::ui {

void LibraryDockWidget::setup_search_interactions() {
    auto* focus_search = new QShortcut{QKeySequence::Find, this};
    focus_search->setContext(Qt::WidgetWithChildrenShortcut);
    QObject::connect(focus_search, &QShortcut::activated, this, [this] {
        search_->setFocus(Qt::ShortcutFocusReason);
        search_->selectAll();
    });
    QObject::connect(search_, &QLineEdit::textChanged, this, [this] { apply_filter(); });
}

}  // namespace shekinah::ui
