#include "shekinah/ui/program_status_widget.hpp"
#include "program_status_components.hpp"

#include "shekinah/media/slide_renderer.hpp"
#include "shekinah/ui/cue_grid_widget.hpp"
#include "shekinah/ui/support/elided_label.hpp"
#include "shekinah/ui/support/theme.hpp"

#include <QAbstractItemView>
#include <QFrame>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMenu>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QProgressBar>
#include <QPushButton>
#include <QScreen>
#include <QScrollArea>
#include <QStackedWidget>
#include <QVBoxLayout>

#include <algorithm>
#include <cmath>

namespace shekinah::ui {
void ProgramStatusWidget::set_program_title(QString title) {
    const auto trimmed = title.trimmed();
    set_slide_preview(trimmed.isEmpty() ? QStringLiteral("No cue live") : trimmed);
}

void ProgramStatusWidget::set_slide_preview(QString text,
                                            QString background_path,
                                            domain::FitMode background_fit) {
    auto* preview = static_cast<ProgramSlidePreview*>(slide_preview_);
    preview->set_content(std::move(text), std::move(background_path), background_fit);
}

void ProgramStatusWidget::set_program(const domain::ProgramState& program) {
    current_program_ = program;
    auto* preview = static_cast<ProgramSlidePreview*>(slide_preview_);
    preview->set_mode(program.mode);
    mode_label_->setText(mode_text(program.mode));
    cue_label_->setText(program.live_cue.has_value()
                            ? QString::fromStdString(program.live_cue->cue.value)
                            : QStringLiteral("No cue live"));
    if (!program.live_cue.has_value()) {
        set_slide_preview({});
        live_details_ = {};
        set_live_details(live_details_);
    }
    update_status_chrome();
}

void ProgramStatusWidget::set_live_details(LiveSlideDetails details) {
    live_details_ = std::move(details);
    const auto& p = ThemeManager::instance().palette();

    const bool has_live =
        current_program_.mode == domain::ProgramMode::cue && current_program_.live_cue.has_value();

    if (!has_live && live_details_.slide_text.isEmpty() && live_details_.item_title.isEmpty()) {
        live_section_badge_->setText(
            current_program_.mode == domain::ProgramMode::black ? QStringLiteral("BLACK")
                                                                : QStringLiteral("IDLE"));
        live_section_badge_->setStyleSheet(
            QStringLiteral("background-color: %1; color: %2; border-radius: 3px; padding: 2px 6px; font-size: 10px; font-weight: 700;")
                .arg(p.badge_bg.name(), p.badge_fg.name()));
        live_item_title_label_->setText(
            active_item_title_.isEmpty() ? QStringLiteral("Ready for service") : active_item_title_);
        live_slide_counter_label_->setText(QStringLiteral("--"));
        live_lyrics_label_->setText(
            QStringLiteral("Click any cue in the grid to project it live to the Audience Output."));
        live_asset_label_->setText(QStringLiteral("Background: None"));
        slide_progress_->setValue(0);
    } else {
        const QString section_label = live_details_.section.trimmed().isEmpty()
                                          ? QStringLiteral("SLIDE")
                                          : live_details_.section.trimmed().toUpper();
        const QColor section_color = CueGridWidget::footer_color_for_section(live_details_.section);
        live_section_badge_->setText(section_label);
        live_section_badge_->setStyleSheet(
            QStringLiteral("background-color: %1; color: #ffffff; border-radius: 3px; padding: 2px 6px; font-size: 10px; font-weight: 700;")
                .arg(section_color.name()));
        live_item_title_label_->setText(
            live_details_.item_title.isEmpty() ? QStringLiteral("Live Cue") : live_details_.item_title);
        if (live_details_.total_slides > 0 && live_details_.slide_index > 0) {
            live_slide_counter_label_->setText(
                QStringLiteral("%1 / %2").arg(live_details_.slide_index).arg(live_details_.total_slides));
            const int pct = std::clamp(
                static_cast<int>(std::lround(100.0 * static_cast<double>(live_details_.slide_index) /
                                             static_cast<double>(live_details_.total_slides))),
                0,
                100);
            slide_progress_->setValue(pct);
        } else {
            live_slide_counter_label_->setText(QStringLiteral("LIVE"));
            slide_progress_->setValue(100);
        }
        live_lyrics_label_->setText(
            live_details_.slide_text.trimmed().isEmpty()
                ? QStringLiteral("Media / image slide active")
                : live_details_.slide_text.trimmed());
        if (live_details_.has_background) {
            const QString kind = live_details_.is_video ? QStringLiteral("Video")
                                                        : fit_mode_text(live_details_.fit_mode);
            live_asset_label_->setText(
                live_details_.asset_name.isEmpty()
                    ? QStringLiteral("Background: Active (%1)").arg(kind)
                    : QStringLiteral("Bg: %1 Â· %2").arg(live_details_.asset_name, kind));
        } else {
            live_asset_label_->setText(QStringLiteral("Background: None"));
        }
    }

    if (!live_details_.next_summary.trimmed().isEmpty() || !live_details_.next_section.trimmed().isEmpty()) {
        const QString next_sec = live_details_.next_section.trimmed().isEmpty()
                                     ? QStringLiteral("NEXT")
                                     : live_details_.next_section.trimmed().toUpper();
        const QColor next_color = CueGridWidget::footer_color_for_section(live_details_.next_section);
        next_section_badge_->setText(next_sec);
        next_section_badge_->setStyleSheet(
            QStringLiteral("background-color: %1; color: #ffffff; border-radius: 3px; padding: 2px 6px; font-size: 10px; font-weight: 700;")
                .arg(next_color.name()));
        next_summary_label_->setText(
            live_details_.next_summary.trimmed().isEmpty()
                ? QStringLiteral("Media / Sermon slide")
                : live_details_.next_summary.trimmed());
        next_up_action_button_->setText(QStringLiteral("Go to Next Slide"));
    } else {
        next_section_badge_->setText(has_live ? QStringLiteral("END") : QStringLiteral("STANDBY"));
        next_section_badge_->setStyleSheet(
            QStringLiteral("background-color: %1; color: %2; border-radius: 3px; padding: 2px 6px; font-size: 10px; font-weight: 700;")
                .arg(p.badge_bg.name(), p.badge_fg.name()));
        next_summary_label_->setText(
            has_live
                ? QStringLiteral("End of current presentation item.")
                : QStringLiteral("Select a Song or Sermon in the Service Order to cue slides."));
        next_up_action_button_->setText(QStringLiteral("Show Audience Window"));
    }
    update_status_chrome();
}

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
