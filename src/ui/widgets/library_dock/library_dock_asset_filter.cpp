#include "shekinah/ui/library_dock_widget.hpp"

#include "shekinah/ui/support/flow_layout.hpp"

#include <QButtonGroup>
#include <QHBoxLayout>
#include <QPushButton>

namespace shekinah::ui {

void LibraryDockWidget::setup_asset_filter(QWidget* toolbar, FlowLayout* toolbar_layout) {
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
