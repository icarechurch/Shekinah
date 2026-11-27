#include "shekinah/ui/library_dock_widget.hpp"

#include "library_components.hpp"

#include "shekinah/ui/support/flow_layout.hpp"

#include <QListWidget>

namespace shekinah::ui {
using namespace library_internal;

void LibraryDockWidget::setup_sermon_toolbar_actions(QWidget* toolbar, FlowLayout* toolbar_layout) {
    const QColor neutral_icon{QStringLiteral("#cfd3dc")};
    auto [group, layout] = create_toolbar_action_group(toolbar, toolbar_layout);
    new_sermon_button_ = add_toolbar_action(group,
                                            layout,
                                            QStringLiteral("New Sermon"),
                                            QStringLiteral("new-sermon"),
                                            QStringLiteral("Create a new Sermon slide deck"),
                                            LibraryGlyph::plus,
                                            neutral_icon,
                                            [this] {
                                                if (new_sermon_handler_) {
                                                    new_sermon_handler_();
                                                }
                                            });
    import_sermon_button_ = add_toolbar_action(group,
                                               layout,
                                               QStringLiteral("Import Deck"),
                                               QStringLiteral("import-sermon-deck"),
                                               QStringLiteral("Import Sermon slide media"),
                                               LibraryGlyph::import_doc,
                                               neutral_icon,
                                               [this] {
                                                   if (import_sermon_handler_) {
                                                       import_sermon_handler_();
                                                   }
                                               });
    edit_sermon_button_ = add_toolbar_action(group,
                                             layout,
                                             QStringLiteral("Edit"),
                                             QStringLiteral("edit-sermon"),
                                             QStringLiteral("Edit selected Sermon deck"),
                                             LibraryGlyph::edit,
                                             neutral_icon,
                                             [this] {
                                                 if (edit_sermon_handler_) {
                                                     edit_sermon_row(sermons_->currentRow());
                                                 }
                                             });
}

}  // namespace shekinah::ui
