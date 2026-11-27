#include "shekinah/ui/program_status_widget.hpp"

#include <QLineEdit>
#include <QListWidget>

namespace shekinah::ui {

void ProgramStatusWidget::apply_cue_filter() {
    const QString query = cues_filter_input_->text().trimmed();
    for (int i = 0; i < cues_list_->count(); ++i) {
        auto* item = cues_list_->item(i);
        const bool matches = query.isEmpty() ||
                             item->text().contains(query, Qt::CaseInsensitive) ||
                             item->toolTip().contains(query, Qt::CaseInsensitive);
        item->setHidden(!matches);
    }
}

}  // namespace shekinah::ui
