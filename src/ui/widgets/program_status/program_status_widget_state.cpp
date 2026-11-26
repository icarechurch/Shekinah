#include "shekinah/ui/program_status_widget.hpp"

#include "shekinah/ui/support/theme.hpp"

#include <QLabel>
#include <QListWidget>
#include <QProgressBar>

#include <algorithm>
#include <cmath>

namespace shekinah::ui {

void ProgramStatusWidget::set_item_cues(QString item_title, std::vector<QuickCueItem> cues) {
    active_item_title_ = std::move(item_title);
    item_cues_ = std::move(cues);
    cues_list_->clear();
    for (const auto& cue : item_cues_) {
        const QString num = cue.number > 0 ? QString::number(cue.number) : QStringLiteral("â€¢");
        const QString sec = cue.section.trimmed().isEmpty() ? QStringLiteral("Slide") : cue.section.trimmed();
        QString first_line = cue.summary.section(QLatin1Char('\n'), 0, 0).trimmed();
        if (first_line.isEmpty()) {
            first_line = QStringLiteral("(media slide)");
        }
        auto* item = new QListWidgetItem{QStringLiteral("%1. [%2] %3").arg(num, sec, first_line)};
        item->setData(Qt::UserRole, QString::fromStdString(cue.id.value));
        item->setToolTip(cue.summary);
        cues_list_->addItem(item);
    }
    cues_count_label_->setText(
        QStringLiteral("%1 CUE%2")
            .arg(static_cast<qulonglong>(item_cues_.size()))
            .arg(item_cues_.size() == 1 ? QString() : QStringLiteral("S")));
    apply_cue_filter();
}

void ProgramStatusWidget::set_output_status(QString target_name, QString mode_label, bool output_visible) {
    if (!target_name.trimmed().isEmpty()) {
        output_target_label_->setText(target_name);
    }
    if (!mode_label.trimmed().isEmpty()) {
        audience_mode_label_->setText(mode_label);
    }
    output_visible_ = output_visible;
    update_status_chrome();
}

void ProgramStatusWidget::set_memory_status(std::size_t working_set_bytes, bool live_media_admitted) {
    last_working_set_bytes_ = working_set_bytes;
    last_live_media_admitted_ = live_media_admitted;
    const auto& p = ThemeManager::instance().palette();
    const double mb = static_cast<double>(working_set_bytes) / (1024.0 * 1024.0);
    const int mb_int = static_cast<int>(std::lround(mb));
    memory_bar_->setValue(std::clamp(mb_int, 0, 300));
    const QString chunk_color = !live_media_admitted
                                    ? p.danger_fg.name()
                                    : (mb >= 220.0 ? p.warning_fg.name() : p.live_fg.name());
    memory_bar_->setStyleSheet(
        QStringLiteral(
            "QProgressBar { background-color: %1; border: 1px solid %2; border-radius: 3px; }"
            "QProgressBar::chunk { background-color: %3; border-radius: 2px; }")
            .arg(p.input_bg.name(), p.border_subtle.name(), chunk_color));
    memory_status_label_->setText(
        QStringLiteral("Memory: %1 MB / 270 MB Â· %2")
            .arg(mb, 0, 'f', 1)
            .arg(live_media_admitted ? QStringLiteral("Live Video Admitted")
                                     : QStringLiteral("Live Video Throttled")));
}

bool ProgramStatusWidget::global_controls_below_status() const noexcept {
    return global_controls_->geometry().top() > cue_label_->geometry().bottom();
}

}  // namespace shekinah::ui
