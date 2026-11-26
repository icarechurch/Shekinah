#include "shekinah/ui/library_dock_widget.hpp"

#include "library_components.hpp"

#include "shekinah/ui/support/flow_layout.hpp"
#include "shekinah/ui/support/theme.hpp"

#include <QButtonGroup>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>

namespace shekinah::ui {
using namespace library_internal;

void LibraryDockWidget::setup_toolbar_navigation(QWidget* toolbar, FlowLayout* toolbar_layout) {
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
}

}  // namespace shekinah::ui
