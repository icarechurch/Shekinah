#include "shekinah/ui/library_dock_widget.hpp"

#include "library_components.hpp"

#include "shekinah/ui/support/elided_label.hpp"
#include "shekinah/ui/support/flow_layout.hpp"

#include <QButtonGroup>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>

namespace shekinah::ui {
using namespace library_internal;

void LibraryDockWidget::setup_title_bar() {
    auto* title_bar = new QWidget{this};
    title_bar->setObjectName(QStringLiteral("library-dock-header"));
    title_bar->setFixedHeight(26);
    auto* title_layout = new QHBoxLayout(title_bar);
    title_layout->setContentsMargins(10, 0, 6, 0);
    title_layout->setSpacing(6);

    auto* title_label = new QLabel{QStringLiteral("LIBRARY"), title_bar};
    title_label->setObjectName(QStringLiteral("library-dock-title"));
    title_layout->addWidget(title_label);

    header_count_badge_ = new QLabel{QStringLiteral("0 ITEMS"), title_bar};
    header_count_badge_->setObjectName(QStringLiteral("library-total-count"));
    header_count_badge_->setFixedHeight(18);
    header_count_badge_->setAlignment(Qt::AlignCenter);
    title_layout->addWidget(header_count_badge_, 0, Qt::AlignVCenter);

    auto* hint_label = new ElidedLabel{
        QStringLiteral("Drag Assets to Slides/Orders  Ã‚Â·  Double-click Song/Sermon to add"),
        title_bar};
    hint_label->setObjectName(QStringLiteral("library-dock-hint"));
    hint_label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    title_layout->addWidget(hint_label, 1);

    float_toggle_button_ = new QPushButton{title_bar};
    float_toggle_button_->setObjectName(QStringLiteral("library-float-toggle"));
    float_toggle_button_->setFixedSize(20, 20);
    float_toggle_button_->setCursor(Qt::PointingHandCursor);
    float_toggle_button_->setToolTip(QStringLiteral("Float or Dock Library"));
    float_toggle_button_->setIconSize(QSize{12, 12});
    QObject::connect(float_toggle_button_, &QPushButton::clicked, this, [this] {
        setFloating(!isFloating());
    });
    title_layout->addWidget(float_toggle_button_);
    setTitleBarWidget(title_bar);
}

void LibraryDockWidget::setup_toolbar(QWidget* container) {
    auto* toolbar = new QWidget{container};
    toolbar->setObjectName(QStringLiteral("library-toolbar"));
    QSizePolicy toolbar_policy{QSizePolicy::Preferred, QSizePolicy::Preferred};
    toolbar_policy.setHeightForWidth(true);
    toolbar->setSizePolicy(toolbar_policy);
    auto* toolbar_layout = new FlowLayout{toolbar, 0, 6, 4};
    toolbar_layout->setContentsMargins(8, 4, 8, 4);

    auto* tab_bar = new QWidget{toolbar};
    tab_bar->setObjectName(QStringLiteral("library-tabs"));
    tab_bar->setFixedHeight(26);
    auto* tab_layout = new QHBoxLayout(tab_bar);
    tab_layout->setContentsMargins(2, 2, 2, 2);
    tab_layout->setSpacing(2);

    tab_group_ = new QButtonGroup{this};
    tab_group_->setExclusive(true);
    const auto make_tab_btn = [&](const QString& label,
                                  const QString& object_name,
                                  LibraryGlyph glyph,
                                  int id) {
        auto* button = new QPushButton{label, tab_bar};
        button->setObjectName(object_name);
        button->setCheckable(true);
        button->setFixedHeight(22);
        button->setCursor(Qt::PointingHandCursor);
        button->setIcon(make_library_icon(glyph, QColor{QStringLiteral("#cfd3dc")}, 12));
        button->setIconSize(QSize{12, 12});
        tab_group_->addButton(button, id);
        tab_layout->addWidget(button);
        return button;
    };
    assets_tab_button_ = make_tab_btn(
        QStringLiteral("Assets (0)"), QStringLiteral("library-tab-assets"), LibraryGlyph::assets_tab, 0);
    songs_tab_button_ = make_tab_btn(
        QStringLiteral("Songs (0)"), QStringLiteral("library-tab-songs"), LibraryGlyph::songs_tab, 1);
    sermons_tab_button_ = make_tab_btn(
        QStringLiteral("Sermons (0)"), QStringLiteral("library-tab-sermons"), LibraryGlyph::sermons_tab, 2);
    assets_tab_button_->setChecked(true);
    QObject::connect(tab_group_, &QButtonGroup::idClicked, this, [this](int id) {
        if (id == 0) {
            show_assets();
        } else if (id == 1) {
            show_songs();
        } else if (id == 2) {
            show_sermons();
        }
    });
    toolbar_layout->addWidget(tab_bar);

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

    asset_filter_bar_ = new QWidget{toolbar};
    asset_filter_bar_->setObjectName(QStringLiteral("asset-filter-bar"));
    asset_filter_bar_->setFixedHeight(26);
    auto* asset_filter_layout = new QHBoxLayout(asset_filter_bar_);
    asset_filter_layout->setContentsMargins(2, 2, 2, 2);
    asset_filter_layout->setSpacing(2);
    asset_filter_group_ = new QButtonGroup{this};
    asset_filter_group_->setExclusive(true);
    const auto make_filter_btn = [&](const QString& label, const QString& object_name, int id) {
        auto* button = new QPushButton{label, asset_filter_bar_};
        button->setObjectName(object_name);
        button->setCheckable(true);
        button->setFixedHeight(22);
        button->setCursor(Qt::PointingHandCursor);
        asset_filter_group_->addButton(button, id);
        asset_filter_layout->addWidget(button);
        return button;
    };
    filter_all_button_ = make_filter_btn(QStringLiteral("All"), QStringLiteral("filter-assets-all"), 0);
    filter_images_button_ =
        make_filter_btn(QStringLiteral("Images"), QStringLiteral("filter-assets-images"), 1);
    filter_videos_button_ =
        make_filter_btn(QStringLiteral("Videos"), QStringLiteral("filter-assets-videos"), 2);
    filter_all_button_->setChecked(true);
    QObject::connect(asset_filter_group_, &QButtonGroup::idClicked, this, [this](int id) {
        if (id == 0) {
            set_asset_kind_filter(std::nullopt);
        } else if (id == 1) {
            set_asset_kind_filter(domain::AssetKind::image);
        } else if (id == 2) {
            set_asset_kind_filter(domain::AssetKind::video);
        }
    });
    toolbar_layout->addWidget(asset_filter_bar_);

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
    static_cast<QVBoxLayout*>(container->layout())->addWidget(toolbar);
}

}  // namespace shekinah::ui
