#include "shekinah/ui/operator_page.hpp"

#include "shekinah/ui/cue_grid_widget.hpp"
#include "shekinah/ui/inspector_dialog.hpp"
#include "shekinah/ui/program_status_widget.hpp"
#include "shekinah/ui/service_order_widget.hpp"

#include <QAction>
#include <QMenu>
#include <QSplitter>
#include <QVBoxLayout>

namespace shekinah::ui {

OperatorPage::OperatorPage(QWidget* parent) : QWidget(parent) {
    setObjectName(QStringLiteral("operator-page"));
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(8, 8, 8, 8);

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
    workspace->setStretchFactor(0, 0);
    workspace->setStretchFactor(1, 1);
    layout->addWidget(workspace, 1);

    program_status_ = new ProgramStatusWidget{this};
    layout->addWidget(program_status_, 0);
    inspector_ = new InspectorDialog{this};
}

void OperatorPage::set_service_order(std::vector<ServiceOrderView> items) {
    service_order_->set_items(std::move(items));
}

void OperatorPage::set_service_order_selection_handler(
    std::function<void(domain::ServiceOrderItemId)> handler) {
    service_order_->set_selection_handler(std::move(handler));
}

void OperatorPage::set_take_live_handler(std::function<void(domain::CueId)> handler) {
    cue_grid_->set_take_live_handler(std::move(handler));
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
    menu->addAction(QStringLiteral("Edit slide"));
    menu->addAction(QStringLiteral("Set background"));
    menu->addAction(QStringLiteral("Duplicate"));
    menu->addAction(QStringLiteral("Delete"));
    QObject::connect(inspector, &QAction::triggered, this, [this, cue] {
        inspector_->show_inspector(domain::ContentRef::song("selected"), cue);
    });
    return menu;
}

}  // namespace shekinah::ui
