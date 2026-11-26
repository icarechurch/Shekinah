#include "shekinah/ui/library_dock_widget.hpp"

#include "library_components.hpp"

namespace shekinah::ui {
using namespace library_internal;

LibraryDockWidget::LibraryDockWidget(QWidget* parent)
    : QDockWidget(parent),
      shared_video_icon_(
          make_library_icon(LibraryGlyph::video_media, QColor{QStringLiteral("#a78bfa")}, 32)),
      thumbnail_cache_(kThumbnailCacheCapKb) {
    setObjectName(QStringLiteral("library-dock"));
    setWindowTitle(QStringLiteral("Library"));
    setAllowedAreas(Qt::BottomDockWidgetArea | Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable |
                QDockWidget::DockWidgetClosable);

    // 1. Sleek custom dock title bar (keeps dock draggable and floatable)
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
        QStringLiteral("Drag Assets to Slides/Orders  Â·  Double-click Song/Sermon to add"),
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

    // 2. Main container
    auto* container = new QWidget{this};
    auto* container_layout = new QVBoxLayout(container);
    container_layout->setContentsMargins(0, 0, 0, 0);
    container_layout->setSpacing(0);

    // 3. Responsive ProPresenter toolbar (Tabs + Search + Media Filters + Grouped Actions)
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

    container_layout->addWidget(toolbar);

    // 4. Stacked content views + single shared lightweight empty-state overlay
    stack_ = new QStackedWidget{container};
    stack_->setObjectName(QStringLiteral("library-stack"));

    assets_ = new AssetListWidget{stack_};
    assets_->setObjectName(QStringLiteral("assets-list"));
    assets_->setViewMode(QListView::IconMode);
    assets_->setFlow(QListView::LeftToRight);
    assets_->setWrapping(true);
    assets_->setResizeMode(QListView::Adjust);
    assets_->setMovement(QListView::Static);
    assets_->setSelectionMode(QAbstractItemView::SingleSelection);
    assets_->setDragEnabled(true);
    assets_->setDragDropMode(QAbstractItemView::DragOnly);
    assets_->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    assets_->setMouseTracking(true);
    assets_->setSpacing(4);
    assets_->setIconSize(QSize{kThumbnailWidth, kThumbnailHeight});
    assets_->setGridSize(QSize{kAssetGridWidth, kAssetGridHeight});
    assets_->setUniformItemSizes(true);
    assets_->setItemDelegate(new AssetCardDelegate{assets_});

    auto* songs_list = new ContentListWidget{domain::ContentKind::song, stack_};
    auto* sermons_list = new ContentListWidget{domain::ContentKind::sermon, stack_};
    songs_ = songs_list;
    sermons_ = sermons_list;
    songs_->setObjectName(QStringLiteral("songs-list"));
    sermons_->setObjectName(QStringLiteral("sermons-list"));

    for (auto* list : {songs_, sermons_}) {
        list->setViewMode(QListView::ListMode);
        list->setFlow(QListView::LeftToRight);
        list->setWrapping(true);
        list->setResizeMode(QListView::Adjust);
        list->setMovement(QListView::Static);
        list->setSelectionMode(QAbstractItemView::SingleSelection);
        list->setDragEnabled(true);
        list->setDragDropMode(QAbstractItemView::DragOnly);
        list->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
        list->setMouseTracking(true);
        list->setSpacing(4);
        list->setGridSize(QSize{kContentGridWidth, kContentGridHeight});
        list->setUniformItemSizes(true);
        list->setItemDelegate(new ContentCardDelegate{list});
    }

    songs_list->set_action_handlers(
        [this](QListWidgetItem* item) {
            if (item != nullptr && add_content_handler_) {
                add_content_handler_(
                    domain::ContentRef::song(item->data(kContentIdRole).toString().toStdString()));
            }
        },
        [this](QListWidgetItem* item) {
            if (item != nullptr && edit_song_handler_) {
                edit_song_handler_(domain::SongId{item->data(kContentIdRole).toString().toStdString()});
            }
        });

    sermons_list->set_action_handlers(
        [this](QListWidgetItem* item) {
            if (item != nullptr && add_content_handler_) {
                add_content_handler_(
                    domain::ContentRef::sermon(item->data(kContentIdRole).toString().toStdString()));
            }
        },
        [this](QListWidgetItem* item) {
            if (item != nullptr && edit_sermon_handler_) {
                edit_sermon_handler_(
                    domain::SermonId{item->data(kContentIdRole).toString().toStdString()});
            }
        });

    songs_->setContextMenuPolicy(Qt::CustomContextMenu);
    QObject::connect(songs_, &QListWidget::customContextMenuRequested, this, [this](const QPoint& position) {
        auto* item = songs_->itemAt(position);
        if (item == nullptr) {
            return;
        }
        QMenu menu{this};
        if (add_content_handler_) {
            auto* add = menu.addAction(QStringLiteral("Add to Service Order"));
            QObject::connect(add, &QAction::triggered, this, [this, item] {
                if (add_content_handler_) {
                    add_content_handler_(
                        domain::ContentRef::song(item->data(kContentIdRole).toString().toStdString()));
                }
            });
        }
        if (edit_song_handler_) {
            auto* edit = menu.addAction(QStringLiteral("Edit Song"));
            QObject::connect(edit, &QAction::triggered, this, [this, item] {
                if (edit_song_handler_) {
                    edit_song_handler_(
                        domain::SongId{item->data(kContentIdRole).toString().toStdString()});
                }
            });
        }
        if (!menu.isEmpty()) {
            menu.exec(songs_->viewport()->mapToGlobal(position));
        }
    });

    QObject::connect(songs_, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem* item) {
        if (item != nullptr && add_content_handler_) {
            add_content_handler_(
                domain::ContentRef::song(item->data(kContentIdRole).toString().toStdString()));
        }
    });

    sermons_->setContextMenuPolicy(Qt::CustomContextMenu);
    QObject::connect(sermons_, &QListWidget::customContextMenuRequested, this, [this](const QPoint& position) {
        auto* item = sermons_->itemAt(position);
        if (item == nullptr) {
            return;
        }
        QMenu menu{this};
        if (add_content_handler_) {
            auto* add = menu.addAction(QStringLiteral("Add to Service Order"));
            QObject::connect(add, &QAction::triggered, this, [this, item] {
                if (add_content_handler_) {
                    add_content_handler_(
                        domain::ContentRef::sermon(item->data(kContentIdRole).toString().toStdString()));
                }
            });
        }
        if (edit_sermon_handler_) {
            auto* edit = menu.addAction(QStringLiteral("Edit Sermon"));
            QObject::connect(edit, &QAction::triggered, this, [this, item] {
                if (edit_sermon_handler_) {
                    edit_sermon_handler_(
                        domain::SermonId{item->data(kContentIdRole).toString().toStdString()});
                }
            });
        }
        if (!menu.isEmpty()) {
            menu.exec(sermons_->viewport()->mapToGlobal(position));
        }
    });

    QObject::connect(sermons_, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem* item) {
        if (item != nullptr && add_content_handler_) {
            add_content_handler_(
                domain::ContentRef::sermon(item->data(kContentIdRole).toString().toStdString()));
        }
    });

    QObject::connect(songs_, &QListWidget::currentRowChanged, this, [this](int) {
        update_action_states();
    });
    QObject::connect(sermons_, &QListWidget::currentRowChanged, this, [this](int) {
        update_action_states();
    });

    stack_->addWidget(assets_);
    stack_->addWidget(songs_);
    stack_->addWidget(sermons_);
    container_layout->addWidget(stack_, 1);

    // Single transparent empty-state overlay shared across all 3 views
    empty_overlay_ = new QWidget{stack_};
    empty_overlay_->setObjectName(QStringLiteral("library-empty-state"));
    empty_overlay_->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    auto* empty_layout = new QVBoxLayout(empty_overlay_);
    empty_layout->setContentsMargins(24, 12, 24, 12);
    empty_layout->setSpacing(4);
    empty_layout->addStretch(1);

    empty_icon_label_ = new QLabel{empty_overlay_};
    empty_icon_label_->setAlignment(Qt::AlignCenter);
    empty_layout->addWidget(empty_icon_label_);

    empty_title_label_ = new QLabel{empty_overlay_};
    empty_title_label_->setObjectName(QStringLiteral("library-empty-title"));
    empty_title_label_->setAlignment(Qt::AlignCenter);
    empty_layout->addWidget(empty_title_label_);

    empty_hint_label_ = new QLabel{empty_overlay_};
    empty_hint_label_->setObjectName(QStringLiteral("library-empty-hint"));
    empty_hint_label_->setAlignment(Qt::AlignCenter);
    empty_hint_label_->setWordWrap(true);
    empty_layout->addWidget(empty_hint_label_);
    empty_layout->addStretch(1);

    setWidget(container);

    auto* focus_search = new QShortcut{QKeySequence::Find, this};
    focus_search->setContext(Qt::WidgetWithChildrenShortcut);
    QObject::connect(focus_search, &QShortcut::activated, this, [this] {
        search_->setFocus(Qt::ShortcutFocusReason);
        search_->selectAll();
    });

    QObject::connect(search_, &QLineEdit::textChanged, this, [this] { apply_filter(); });

    set_tab(LibraryTab::assets, 0);
    update_tab_and_header_state();
    apply_theme();
    QObject::connect(&ThemeManager::instance(), &ThemeManager::theme_changed, this,
                     [this](ThemeId) { apply_theme(); });
}

}  // namespace shekinah::ui
