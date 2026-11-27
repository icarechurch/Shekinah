#include "shekinah/ui/library_dock_widget.hpp"

#include "library_components.hpp"

#include "shekinah/ui/support/flow_layout.hpp"
#include "shekinah/ui/support/theme.hpp"

#include <QLineEdit>

namespace shekinah::ui {
using namespace library_internal;

void LibraryDockWidget::setup_search_setup(QWidget* toolbar, FlowLayout* toolbar_layout) {
    search_ = new QLineEdit{toolbar};
    search_->setObjectName(QStringLiteral("library-search"));
    search_->setPlaceholderText(QStringLiteral("Search Assets, Songs, or Sermons..."));
    search_->setClearButtonEnabled(true);
    search_->setFixedHeight(26);
    search_->setMinimumWidth(140);
    search_->setMaximumWidth(230);
    search_action_ = search_->addAction(
        make_library_icon(LibraryGlyph::search, ThemeManager::instance().palette().icon_muted, 12),
        QLineEdit::LeadingPosition);
    toolbar_layout->addWidget(search_);
}

}  // namespace shekinah::ui
