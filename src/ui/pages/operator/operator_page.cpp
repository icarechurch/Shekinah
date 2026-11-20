#include "shekinah/ui/operator_page.hpp"

#include "shekinah/ui/cue_grid_widget.hpp"
#include "shekinah/ui/inspector_dialog.hpp"
#include "shekinah/ui/program_status_widget.hpp"
#include "shekinah/ui/service_order_widget.hpp"

#include <QAction>
#include <QLabel>
#include <QMenu>
#include <QSplitter>
#include <QVBoxLayout>

namespace shekinah::ui {

OperatorPage::OperatorPage(QWidget* parent) : QWidget(parent) {
    setObjectName(QStringLiteral("operator-page"));
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(8, 8, 8, 8);

    auto* presentation_section = new QLabel{QStringLiteral("Presentation"), this};
    presentation_section->setObjectName(QStringLiteral("presentation-section"));
    presentation_section->setStyleSheet(QStringLiteral("font-size: 20px; font-weight: 600;"));
    layout->addWidget(presentation_section);

    auto* workspace = new QSplitter{Qt::Horizontal, this};
    workspace->setObjectName(QStringLiteral("operator-workspace"));
    service_order_ = new ServiceOrderWidget{workspace};
    service_order_->setMinimumWidth(220);
    cue_grid_ = new CueGridWidget{workspace};
    cue_grid_->setObjectName(QStringLiteral("cue-grid"));
    cue_grid_->set_context_handler([this](domain::CueId cue, QPoint position) {
        auto* menu = create_context_menu(cue);
        menu->exec(position);
        menu->deleteLater();
    });
    workspace->addWidget(service_order_);
    workspace->addWidget(cue_grid_);
    program_status_ = new ProgramStatusWidget{workspace};
    program_status_->setMinimumWidth(260);
    program_status_->setMaximumWidth(340);
    workspace->addWidget(program_status_);
    workspace->setCollapsible(0, false);
    workspace->setCollapsible(1, false);
    workspace->setCollapsible(2, false);
    workspace->setStretchFactor(0, 0);
    workspace->setStretchFactor(1, 1);
    workspace->setStretchFactor(2, 0);
    workspace->setSizes({220, 760, 280});
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

void OperatorPage::set_authoring_handlers(std::function<void()> new_song,
                                          std::function<void()> import_song,
                                          std::function<void()> import_sermon,
                                          std::function<void()> import_asset) {
    cue_grid_->set_authoring_handlers(
        std::move(new_song), std::move(import_song), std::move(import_sermon), std::move(import_asset));
}

void OperatorPage::set_inspector_handler(std::function<void(domain::CueId)> handler) {
    inspector_handler_ = std::move(handler);
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
    cue_grid_->set_take_live_handler(std::move(handler));
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

QMenu* OperatorPage::create_context_menu(const domain::CueId& cue) {
    auto* menu = new QMenu{this};
    auto* inspector = menu->addAction(QStringLiteral("Open Inspector"));
    auto* edit = menu->addAction(QStringLiteral("Edit slide"));
    auto* background = menu->addAction(QStringLiteral("Set background"));
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
