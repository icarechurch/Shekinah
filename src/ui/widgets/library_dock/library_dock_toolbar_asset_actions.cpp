#include "shekinah/ui/library_dock_widget.hpp"

#include "library_components.hpp"

#include "shekinah/ui/support/flow_layout.hpp"

namespace shekinah::ui {
using namespace library_internal;

void LibraryDockWidget::setup_asset_toolbar_actions(QWidget* toolbar, FlowLayout* toolbar_layout) {
    const QColor neutral_icon{QStringLiteral("#cfd3dc")};
    auto [group, layout] = create_toolbar_action_group(toolbar, toolbar_layout);
    import_asset_button_ = add_toolbar_action(
        group,
        layout,
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
}

}  // namespace shekinah::ui
