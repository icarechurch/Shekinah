#include "shekinah/ui/inspector_dialog.hpp"

#include <QComboBox>
#include <QLineEdit>
#include <QPlainTextEdit>

#include <utility>

namespace shekinah::ui {

void InspectorDialog::emit_save() {
    if (!save_handler_ || cue_.empty()) {
        return;
    }

    std::vector<std::string> lines;
    const auto values = lyrics_->toPlainText().split(QLatin1Char('\n'), Qt::KeepEmptyParts);
    lines.reserve(static_cast<std::size_t>(values.size()));
    for (const auto& value : values) {
        lines.push_back(value.toStdString());
    }
    const auto fit = fit_mode_->currentIndex() == 0
                         ? domain::FitMode::contain
                         : fit_mode_->currentIndex() == 1 ? domain::FitMode::cover : domain::FitMode::stretch;
    save_handler_(cue_, section_->text().trimmed().toStdString(), std::move(lines), fit);
}

}  // namespace shekinah::ui
