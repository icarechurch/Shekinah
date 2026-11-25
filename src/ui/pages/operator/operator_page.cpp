#include "shekinah/ui/operator_page.hpp"

#include "shekinah/ui/cue_grid_widget.hpp"
#include "shekinah/ui/inspector_dialog.hpp"
#include "shekinah/ui/program_status_widget.hpp"
#include "shekinah/ui/service_order_widget.hpp"

#include <QAction>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QSplitter>
#include <QVBoxLayout>

namespace shekinah::ui {

OperatorPage::OperatorPage(QWidget* parent) : QWidget(parent) {
    setObjectName(QStringLiteral("operator-page"));
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    auto* workspace = new QSplitter{Qt::Horizontal, this};
    workspace->setObjectName(QStringLiteral("operator-workspace"));
    workspace->setHandleWidth(2);

    service_order_ = new ServiceOrderWidget{workspace};
    service_order_->setMinimumWidth(220);
    service_order_->setMaximumWidth(380);

    auto* center_pane = new QWidget{workspace};
    center_pane->setObjectName(QStringLiteral("presentation-pane"));
    auto* center_layout = new QVBoxLayout(center_pane);
    center_layout->setContentsMargins(0, 0, 0, 0);
    center_layout->setSpacing(0);

    auto* presentation_header = new QWidget{center_pane};
    presentation_header->setObjectName(QStringLiteral("presentation-section"));
    presentation_header->setFixedHeight(26);
    presentation_header->setStyleSheet(QStringLiteral(
        "#presentation-section { background-color: #191a1e; border-bottom: 1px solid #111215; }"));
    auto* presentation_header_layout = new QHBoxLayout(presentation_header);
    presentation_header_layout->setContentsMargins(10, 0, 10, 0);
    presentation_header_layout->setSpacing(8);

    auto* presentation_title = new QLabel{QStringLiteral("PRESENTATION"), presentation_header};
    presentation_title->setStyleSheet(QStringLiteral(
        "color: #8e929b; font-size: 10px; font-weight: 700; letter-spacing: 1.1px; border: none;"));
    presentation_header_layout->addWidget(presentation_title);

    presentation_item_label_ = new QLabel{QString{}, presentation_header};
    presentation_item_label_->setObjectName(QStringLiteral("presentation-active-item"));
    presentation_item_label_->setStyleSheet(QStringLiteral(
        "color: #d7dae0; font-size: 11px; font-weight: 600; border: none;"));
    presentation_item_label_->setVisible(false);
    presentation_header_layout->addWidget(presentation_item_label_, 1);
    center_layout->addWidget(presentation_header);

    cue_grid_ = new CueGridWidget{center_pane};
    cue_grid_->setObjectName(QStringLiteral("cue-grid"));
    cue_grid_->set_context_handler([this](domain::CueId cue, QPoint position) {
        auto* menu = create_context_menu(cue);
        menu->exec(position);
        menu->deleteLater();
    });
    center_layout->addWidget(cue_grid_, 1);

    workspace->addWidget(service_order_);
    workspace->addWidget(center_pane);
    program_status_ = new ProgramStatusWidget{workspace};
    program_status_->setMinimumWidth(290);
    program_status_->setMaximumWidth(400);
    workspace->addWidget(program_status_);
    workspace->setCollapsible(0, false);
    workspace->setCollapsible(1, false);
    workspace->setCollapsible(2, false);
    workspace->setStretchFactor(0, 0);
    workspace->setStretchFactor(1, 1);
    workspace->setStretchFactor(2, 0);
    workspace->setSizes({248, 712, 320});
    layout->addWidget(workspace, 1);
    inspector_ = new InspectorDialog{this};
}

void OperatorPage::set_service_order(std::vector<ServiceOrderView> items) {
    service_order_->set_items(std::move(items));
}

void OperatorPage::set_service_order_selection_handler(
    std::function<void(domain::ServiceOrderItemId)> handler) {
    service_order_->set_selection_handler(std::move(handler));
}

void OperatorPage::set_service_order_move_handler(
    std::function<void(domain::ServiceOrderItemId, std::int64_t)> handler) {
    service_order_->set_move_handler(std::move(handler));
}

void OperatorPage::set_service_order_asset_drop_handler(
    std::function<void(domain::ContentRef, domain::AssetId)> handler) {
    service_order_->set_asset_drop_handler(std::move(handler));
}

void OperatorPage::set_service_order_content_drop_handler(
    std::function<void(domain::ContentRef)> handler) {
    service_order_->set_content_drop_handler(std::move(handler));
}

void OperatorPage::set_service_order_remove_handler(
    std::function<void(domain::ServiceOrderItemId)> handler) {
    service_order_->set_remove_handler(std::move(handler));
}

void OperatorPage::set_service_order_edit_handler(
    std::function<void(domain::ContentRef)> handler) {
    service_order_->set_edit_handler(std::move(handler));
}

void OperatorPage::set_service_order_new_sermon_handler(std::function<void()> handler) {
    new_sermon_handler_ = std::move(handler);
    service_order_->set_authoring_handlers(
        new_song_handler_, import_song_handler_, new_sermon_handler_, import_sermon_handler_);
}

void OperatorPage::set_service_order_live_content(std::optional<domain::ContentRef> live_content) {
    service_order_->set_live_content(std::move(live_content));
}

void OperatorPage::set_active_presentation_title(const QString& title) {
    const QString trimmed = title.trimmed();
    if (trimmed.isEmpty()) {
        presentation_item_label_->clear();
        presentation_item_label_->setVisible(false);
    } else {
        presentation_item_label_->setText(QStringLiteral("·  %1").arg(trimmed));
        presentation_item_label_->setVisible(true);
    }
}

void OperatorPage::set_cue_asset_drop_handler(
    std::function<void(domain::CueId, domain::AssetId)> handler) {
    cue_grid_->set_asset_drop_handler(std::move(handler));
}

void OperatorPage::set_authoring_handlers(std::function<void()> new_song,
                                          std::function<void()> import_song,
                                          std::function<void()> import_sermon,
                                          std::function<void()> import_asset) {
    new_song_handler_ = new_song;
    import_song_handler_ = import_song;
    import_sermon_handler_ = import_sermon;
    service_order_->set_authoring_handlers(
        new_song_handler_, import_song_handler_, new_sermon_handler_, import_sermon_handler_);
    cue_grid_->set_authoring_handlers(
        std::move(new_song), std::move(import_song), std::move(import_sermon), std::move(import_asset));
}

void OperatorPage::set_inspector_handler(std::function<void(domain::CueId)> handler) {
    inspector_handler_ = std::move(handler);
}

void OperatorPage::set_cue_asset_name_handler(std::function<QString(domain::CueId)> handler) {
    cue_asset_name_handler_ = std::move(handler);
}

void OperatorPage::set_remove_asset_handler(std::function<void(domain::CueId)> handler) {
    remove_asset_handler_ = std::move(handler);
}

void OperatorPage::set_edit_slide_handler(std::function<void(domain::CueId)> handler) {
    edit_slide_handler_ = std::move(handler);
}

void OperatorPage::set_duplicate_slide_handler(std::function<void(domain::CueId)> handler) {
    duplicate_slide_handler_ = std::move(handler);
}

void OperatorPage::set_delete_slide_handler(std::function<void(domain::CueId)> handler) {
    delete_slide_handler_ = std::move(handler);
}

void OperatorPage::set_inspector_save_handler(InspectorDialog::SaveHandler handler) {
    inspector_->set_save_handler(std::move(handler));
}

void OperatorPage::set_take_live_handler(std::function<void(domain::CueId)> handler) {
    cue_grid_->set_take_live_handler(handler);
    program_status_->set_take_cue_handler(std::move(handler));
}

void OperatorPage::set_background_handler(std::function<void(domain::CueId)> handler) {
    background_handler_ = std::move(handler);
}

void OperatorPage::set_previous_handler(std::function<void()> handler) {
    program_status_->set_previous_handler(std::move(handler));
}

void OperatorPage::set_next_handler(std::function<void()> handler) {
    program_status_->set_next_handler(std::move(handler));
}

void OperatorPage::set_clear_handler(std::function<void()> handler) {
    program_status_->set_clear_handler(std::move(handler));
}

void OperatorPage::set_black_handler(std::function<void()> handler) {
    program_status_->set_black_handler(std::move(handler));
}

void OperatorPage::set_show_screen_handler(std::function<void(QScreen*)> handler) {
    program_status_->set_show_screen_handler(std::move(handler));
}

void OperatorPage::set_show_movable_handler(std::function<void()> handler) {
    program_status_->set_show_movable_handler(std::move(handler));
}

void OperatorPage::set_hide_output_handler(std::function<void()> handler) {
    program_status_->set_hide_output_handler(std::move(handler));
}

void OperatorPage::set_inspect_live_handler(std::function<void()> handler) {
    program_status_->set_inspect_live_handler(std::move(handler));
}

void OperatorPage::set_choose_background_handler(std::function<void()> handler) {
    program_status_->set_choose_background_handler(std::move(handler));
}

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
