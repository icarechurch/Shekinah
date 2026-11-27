#include "shekinah/ui/reflow_editor_dialog.hpp"

#include "shekinah/ui/reflow_text_edit.hpp"

#include <QListWidget>

namespace shekinah::ui {

int ReflowEditorDialog::thumbnail_count() const noexcept { return thumbnails_->count(); }

void ReflowEditorDialog::refresh_thumbnails() {
    thumbnails_->clear();
    for (const auto& slide : editor_->reflow_document().to_slides()) {
        const auto section = slide.section.empty() ? "Slide" : slide.section;
        const auto summary = slide.lines.empty() ? "" : slide.lines.front();
        thumbnails_->addItem(QStringLiteral("%1 â€” %2")
                                 .arg(QString::fromStdString(section))
                                 .arg(QString::fromStdString(summary)));
    }
}

}  // namespace shekinah::ui
