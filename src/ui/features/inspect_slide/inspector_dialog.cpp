#include "shekinah/ui/inspector_dialog.hpp"

#include <QLabel>
#include <QVBoxLayout>

namespace shekinah::ui {

InspectorDialog::InspectorDialog(QWidget* parent) : QDialog(parent) {
    setObjectName(QStringLiteral("inspector-dialog"));
    setWindowFlag(Qt::Tool, true);
    setModal(false);
    setWindowTitle(QStringLiteral("Inspector"));
    auto* layout = new QVBoxLayout(this);
    auto* description = new QLabel{
        QStringLiteral("Edit this cue without changing the live Program."), this};
    description->setObjectName(QStringLiteral("inspector-description"));
    layout->addWidget(description);
    resize(360, 220);
}

void InspectorDialog::show_inspector(domain::ContentRef content, domain::CueId cue) {
    content_ = std::move(content);
    cue_ = std::move(cue);
    setWindowTitle(QStringLiteral("Inspector — %1").arg(QString::fromStdString(cue_.value)));
    show();
    raise();
    activateWindow();
}

}  // namespace shekinah::ui
