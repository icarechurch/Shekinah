#include "shekinah/ui/operator_page.hpp"

#include "shekinah/ui/cue_grid_widget.hpp"
#include "shekinah/ui/inspector_dialog.hpp"
#include "shekinah/ui/program_status_widget.hpp"
#include "shekinah/ui/service_order_widget.hpp"
#include "shekinah/ui/support/elided_label.hpp"
#include "shekinah/ui/support/theme.hpp"
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
    service_order_->setMinimumWidth(200);
    service_order_->setMaximumWidth(380);

    auto* center_pane = new QWidget{workspace};
    center_pane->setObjectName(QStringLiteral("presentation-pane"));
    auto* center_layout = new QVBoxLayout(center_pane);
    center_layout->setContentsMargins(0, 0, 0, 0);
    center_layout->setSpacing(0);

    auto* presentation_header = new QWidget{center_pane};
    presentation_header->setObjectName(QStringLiteral("presentation-section"));
    presentation_header->setFixedHeight(26);
    auto* presentation_header_layout = new QHBoxLayout(presentation_header);
    presentation_header_layout->setContentsMargins(10, 0, 10, 0);
    presentation_header_layout->setSpacing(8);

    auto* presentation_title = new QLabel{QStringLiteral("PRESENTATION"), presentation_header};
    presentation_title->setObjectName(QStringLiteral("presentation-section-title"));
    presentation_header_layout->addWidget(presentation_title);

    presentation_item_label_ = new ElidedLabel{QString{}, presentation_header};
    presentation_item_label_->setObjectName(QStringLiteral("presentation-active-item"));
    presentation_item_label_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
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
    program_status_->setMinimumWidth(270);
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

    apply_theme();
    QObject::connect(&ThemeManager::instance(), &ThemeManager::theme_changed, this,
                     [this](ThemeId) { apply_theme(); });
}

}  // namespace shekinah::ui
