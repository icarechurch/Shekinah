#include "shekinah/ui/main_window.hpp"

#include "asset_catalog_coordinator.hpp"

#include <QInputDialog>
#include <QStringList>

namespace shekinah::ui {

void ShekinahWindow::choose_background(const domain::CueId& cue) {
    const auto asset_records = asset_catalog_coordinator_->list();
    if (!asset_records || asset_records->empty()) {
        return;
    }
    QStringList labels;
    labels.reserve(static_cast<qsizetype>(asset_records->size()));
    for (const auto& asset : *asset_records) {
        labels.push_back(QString::fromStdString(asset.relative_path));
    }
    bool accepted = false;
    const auto selected = QInputDialog::getItem(
        this, QStringLiteral("Set Cue Background"), QStringLiteral("Asset"), labels, 0, false,
        &accepted);
    if (!accepted) {
        return;
    }
    const auto index = labels.indexOf(selected);
    if (index >= 0) {
        set_slide_background(cue, asset_records->at(static_cast<std::size_t>(index)).id);
    }
}

}  // namespace shekinah::ui
