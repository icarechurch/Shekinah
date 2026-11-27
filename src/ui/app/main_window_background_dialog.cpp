#include "shekinah/ui/main_window.hpp"

#include "shekinah/storage/content_repository.hpp"

#include <QInputDialog>
#include <QStringList>

namespace shekinah::ui {

void ShekinahWindow::choose_background(const domain::CueId& cue) {
    if (!repository()) {
        return;
    }
    const auto assets = repository()->list_assets();
    if (!assets || assets->empty()) {
        return;
    }
    QStringList labels;
    labels.reserve(static_cast<qsizetype>(assets->size()));
    for (const auto& asset : *assets) {
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
        set_slide_background(cue, assets->at(static_cast<std::size_t>(index)).id);
    }
}

}  // namespace shekinah::ui
