#include "shekinah/ui/library_dock_widget.hpp"

#include "library_components.hpp"

#include "shekinah/ui/support/flow_layout.hpp"

#include <QButtonGroup>
#include <QHBoxLayout>
#include <QPushButton>

namespace shekinah::ui {
using namespace library_internal;

void LibraryDockWidget::setup_tab_navigation(QWidget* toolbar, FlowLayout* toolbar_layout) {
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
}

}  // namespace shekinah::ui
