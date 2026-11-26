#include "shekinah/ui/operator_page.hpp"

#include "shekinah/ui/inspector_dialog.hpp"

#include <QAction>
#include <QMenu>

namespace shekinah::ui {

QMenu* OperatorPage::create_context_menu(const domain::CueId& cue) {
    auto* menu = new QMenu{this};
    auto* inspector = menu->addAction(QStringLiteral("Open Inspector"));
    auto* edit = menu->addAction(QStringLiteral("Edit slide"));
    auto* background = menu->addAction(QStringLiteral("Set background"));
    const auto asset_name = cue_asset_name_handler_ ? cue_asset_name_handler_(cue) : QString{};
    QAction* remove_asset = nullptr;
    if (!asset_name.isEmpty()) {
        remove_asset = menu->addAction(QStringLiteral("Remove asset %1").arg(asset_name));
    }
    auto* duplicate = menu->addAction(QStringLiteral("Duplicate"));
    auto* remove = menu->addAction(QStringLiteral("Delete"));
    QObject::connect(inspector, &QAction::triggered, this, [this, cue] {
        if (inspector_handler_) {
            inspector_handler_(cue);
        } else {
            inspector_->show_inspector(domain::ContentRef::song("selected"), cue);
        }
    });
    QObject::connect(edit, &QAction::triggered, this, [this, cue] {
        if (edit_slide_handler_) {
            edit_slide_handler_(cue);
        }
    });
    QObject::connect(background, &QAction::triggered, this, [this, cue] {
        if (background_handler_) {
            background_handler_(cue);
        }
    });
    if (remove_asset != nullptr) {
        QObject::connect(remove_asset, &QAction::triggered, this, [this, cue] {
            if (remove_asset_handler_) {
                remove_asset_handler_(cue);
            }
        });
    }
    QObject::connect(duplicate, &QAction::triggered, this, [this, cue] {
        if (duplicate_slide_handler_) {
            duplicate_slide_handler_(cue);
        }
    });
    QObject::connect(remove, &QAction::triggered, this, [this, cue] {
        if (delete_slide_handler_) {
            delete_slide_handler_(cue);
        }
    });
    return menu;
}

}  // namespace shekinah::ui
