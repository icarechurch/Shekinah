#include "shekinah/ui/library_dock_widget.hpp"

#include "library_components.hpp"

#include <QVBoxLayout>

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

    setup_title_bar();
    auto* container = new QWidget{this};
    auto* container_layout = new QVBoxLayout(container);
    container_layout->setContentsMargins(0, 0, 0, 0);
    container_layout->setSpacing(0);
    setup_toolbar(container);
    setup_content(container);
    setWidget(container);
    setup_interactions();

    set_tab(LibraryTab::assets, 0);
    update_tab_and_header_state();
    apply_theme();
    QObject::connect(&ThemeManager::instance(), &ThemeManager::theme_changed, this,
                     [this](ThemeId) { apply_theme(); });
}

}  // namespace shekinah::ui
