#include "shekinah/ui/library_dock_widget.hpp"

#include "library_components.hpp"

#include "shekinah/ui/support/flow_layout.hpp"

#include <QListWidget>

namespace shekinah::ui {
using namespace library_internal;

void LibraryDockWidget::setup_song_toolbar_actions(QWidget* toolbar, FlowLayout* toolbar_layout) {
    const QColor neutral_icon{QStringLiteral("#cfd3dc")};
    auto [group, layout] = create_toolbar_action_group(toolbar, toolbar_layout);
    new_song_button_ = add_toolbar_action(group,
                                          layout,
                                          QStringLiteral("New Song"),
                                          QStringLiteral("new-song"),
                                          QStringLiteral("Create a new Song in Reflow editor"),
                                          LibraryGlyph::plus,
                                          neutral_icon,
                                          [this] {
                                              if (new_song_handler_) {
                                                  new_song_handler_();
                                              }
                                          });
    import_song_button_ = add_toolbar_action(group,
                                             layout,
                                             QStringLiteral("Import Text"),
                                             QStringLiteral("import-song-text"),
                                             QStringLiteral("Import Song from text file"),
                                             LibraryGlyph::import_doc,
                                             neutral_icon,
                                             [this] {
                                                 if (import_song_handler_) {
                                                     import_song_handler_();
                                                 }
                                             });
    edit_song_button_ = add_toolbar_action(group,
                                           layout,
                                           QStringLiteral("Edit"),
                                           QStringLiteral("edit-song"),
                                           QStringLiteral("Edit selected Song"),
                                           LibraryGlyph::edit,
                                           neutral_icon,
                                           [this] {
                                               if (edit_song_handler_) {
                                                   edit_song_row(songs_->currentRow());
                                               }
                                           });
}

}  // namespace shekinah::ui
