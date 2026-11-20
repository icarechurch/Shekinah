#include "shekinah/ui/operator_page.hpp"

#include "shekinah/ui/cue_grid_widget.hpp"
#include "shekinah/ui/inspector_dialog.hpp"
#include "shekinah/ui/program_status_widget.hpp"

#include <QAction>
#include <QListWidget>
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
    service_order_ = new QListWidget{workspace};
    service_order_->setObjectName(QStringLiteral("service-order"));
    service_order_->setMinimumWidth(220);
    service_order_->addItem(QStringLiteral("Service Order"));
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
