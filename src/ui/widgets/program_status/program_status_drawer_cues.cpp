#include "shekinah/ui/program_status_widget.hpp"

#include "program_status_sections.hpp"

#include "shekinah/ui/support/elided_label.hpp"

#include <QAbstractItemView>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QSizePolicy>
#include <QStackedWidget>
#include <QVBoxLayout>

namespace shekinah::ui {

void ProgramStatusWidget::build_cues_drawer_page() {
    auto* cues_page = new QWidget{drawer_stack_};
    cues_page->setObjectName(QStringLiteral("drawer-page-cues"));
    auto* cues_layout = new QVBoxLayout(cues_page);
    cues_layout->setContentsMargins(0, 0, 0, 0);
    cues_layout->setSpacing(0);

    cues_layout->addWidget(
        make_section_header(QStringLiteral("0 CUES"), cues_page, nullptr, &cues_count_label_));

    cues_list_ = new QListWidget{cues_page};
    cues_list_->setObjectName(QStringLiteral("quick-cues-list"));
    cues_list_->setMinimumHeight(36);
    cues_list_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Ignored);
    cues_list_->setSelectionMode(QAbstractItemView::SingleSelection);
    QObject::connect(cues_list_, &QListWidget::itemClicked, this, [this](QListWidgetItem* item) {
        if (item == nullptr || !take_cue_handler_) {
            return;
        }
        const auto id = item->data(Qt::UserRole).toString();
        if (!id.isEmpty()) {
            take_cue_handler_(domain::CueId{id.toStdString()});
        }
    });
    cues_layout->addWidget(cues_list_, 1);

    auto* filter_bar = new QWidget{cues_page};
    filter_bar->setObjectName(QStringLiteral("cues-filter-bar"));
    auto* filter_layout = new QHBoxLayout(filter_bar);
    filter_layout->setContentsMargins(8, 5, 8, 5);
    filter_layout->setSpacing(6);

    cues_filter_input_ = new QLineEdit{filter_bar};
    cues_filter_input_->setObjectName(QStringLiteral("quick-cues-filter"));
    cues_filter_input_->setMinimumWidth(60);
    cues_filter_input_->setPlaceholderText(QStringLiteral("Filter cues..."));
    cues_filter_input_->setClearButtonEnabled(true);
    QObject::connect(cues_filter_input_, &QLineEdit::textChanged, this, [this] {
        apply_cue_filter();
    });
    filter_layout->addWidget(cues_filter_input_, 1);

    auto* shortcut_hint = new ElidedLabel{QStringLiteral("PgUp/Dn Â· Esc Â· B"), filter_bar};
    shortcut_hint->setObjectName(QStringLiteral("quick-cues-shortcut-hint"));
    filter_layout->addWidget(shortcut_hint);
    cues_layout->addWidget(filter_bar);
    drawer_stack_->addWidget(cues_page);
}

}  // namespace shekinah::ui
