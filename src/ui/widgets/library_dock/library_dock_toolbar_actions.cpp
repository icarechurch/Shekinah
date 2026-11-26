#include "shekinah/ui/library_dock_widget.hpp"

#include "library_components.hpp"

#include "shekinah/ui/support/flow_layout.hpp"

#include <QHBoxLayout>
#include <QListWidget>
#include <QPushButton>

namespace shekinah::ui {
using namespace library_internal;

void LibraryDockWidget::setup_toolbar_actions(QWidget* toolbar, FlowLayout* toolbar_layout) {
    const auto add_action_btn = [&](QWidget* group_parent,
                                    QHBoxLayout* group_layout,
                                    const QString& label,
                                    const QString& object_name,
                                    const QString& tooltip,
                                    LibraryGlyph glyph,
                                    const QColor& icon_color,
                                    auto handler) {
        auto* button = new QPushButton{label, group_parent};
        button->setObjectName(object_name);
        button->setProperty("libAction", true);
        button->setProperty("primaryAction", false);
        button->setFixedHeight(26);
        button->setCursor(Qt::PointingHandCursor);
        button->setToolTip(tooltip);
        button->setIcon(make_library_icon(glyph, icon_color, 12));
        button->setIconSize(QSize{12, 12});
        QObject::connect(button, &QPushButton::clicked, this, handler);
        group_layout->addWidget(button);
        return button;
    };
    const auto make_action_group = [&] {
        auto* group = new QWidget{toolbar};
        group->setStyleSheet(QStringLiteral("background: transparent; border: none;"));
        auto* row = new QHBoxLayout(group);
        row->setContentsMargins(0, 0, 0, 0);
        row->setSpacing(4);
        toolbar_layout->addWidget(group);
        return std::pair{group, row};
    };

    const QColor neutral_icon{QStringLiteral("#cfd3dc")};
    auto [asset_group, asset_group_layout] = make_action_group();
    import_asset_button_ = add_action_btn(asset_group,
                                          asset_group_layout,
                                          QStringLiteral("Import Asset"),
                                          QStringLiteral("import-asset"),
                                          QStringLiteral("Import image or video background"),
                                          LibraryGlyph::import_doc,
                                          neutral_icon,
                                          [this] {
                                              if (import_asset_handler_) {
                                                  import_asset_handler_();
                                              }
                                          });
    auto [song_group, song_group_layout] = make_action_group();
    new_song_button_ = add_action_btn(song_group,
                                      song_group_layout,
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
    import_song_button_ = add_action_btn(song_group,
                                         song_group_layout,
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
    edit_song_button_ = add_action_btn(song_group,
                                       song_group_layout,
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
    auto [sermon_group, sermon_group_layout] = make_action_group();
    new_sermon_button_ = add_action_btn(sermon_group,
                                        sermon_group_layout,
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
    import_sermon_button_ = add_action_btn(sermon_group,
                                           sermon_group_layout,
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
    edit_sermon_button_ = add_action_btn(sermon_group,
                                         sermon_group_layout,
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
