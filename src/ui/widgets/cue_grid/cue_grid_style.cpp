#include "shekinah/ui/cue_grid_widget.hpp"

#include "cue_card.hpp"
#include "cue_grid_overlay.hpp"

#include "shekinah/ui/support/theme.hpp"

#include <algorithm>

namespace shekinah::ui {

void CueGridWidget::apply_theme() {
    const auto& p = ThemeManager::instance().palette();
    setStyleSheet(
        QStringLiteral(
            "QWidget#cue-grid { background-color: %1; }"
            "QScrollArea#presentation-empty-scroll, QScrollArea#cue-grid-scroll,"
            "QWidget#presentation-empty-state {"
            "  background-color: %1;"
            "  color: %2;"
            "}"
            "QLabel#presentation-title {"
            "  color: %2;"
            "  font-size: 22px;"
            "  font-weight: 600;"
            "  background: transparent;"
            "}"
            "QLabel#presentation-empty-message {"
            "  color: %3;"
            "  background: transparent;"
            "}"
            "QWidget#cue-grid QPushButton {"
            "  background-color: %4;"
            "  border: 1px solid %5;"
            "  border-radius: 4px;"
            "  color: %2;"
            "  padding: 5px 10px;"
            "  font-weight: 600;"
            "}"
            "QWidget#cue-grid QPushButton:hover {"
            "  background-color: %6;"
            "  border-color: %7;"
            "}"
            "QWidget#cue-grid QPushButton:pressed {"
            "  background-color: %8;"
            "}")
            .arg(p.window_bg.name(),
                 p.text_primary.name(),
                 p.text_muted.name(),
                 p.button_bg.name(),
                 p.button_border.name(),
                 p.button_hover_bg.name(),
                 p.button_hover_border.name(),
                 p.button_pressed_bg.name()));
    if (grid_container_ != nullptr) {
        grid_container_->setStyleSheet(
            QStringLiteral("background-color: %1;").arg(p.window_bg.name()));
    }
    if (insert_overlay_ != nullptr) {
        insert_overlay_->update();
    }
    for (auto* card : cards_) {
        if (card != nullptr) {
            card->update();
        }
    }
    update();
}

QColor CueGridWidget::footer_color_for_section(const QString& section) {
    const auto normalized = section.trimmed().toLower();
    if (normalized.startsWith(QStringLiteral("chorus"))) {
        return QColor{QStringLiteral("#cc1f58")};
    }
    if (normalized.startsWith(QStringLiteral("bridge"))) {
        return QColor{QStringLiteral("#8336c7")};
    }
    if (normalized.startsWith(QStringLiteral("tag"))) {
        return QColor{QStringLiteral("#cf3a32")};
    }
    return QColor{QStringLiteral("#0b80ba")};
}

int CueGridWidget::column_count_for_geometry(int viewport_width, int screen_width) {
    int screen_columns = 6;
    if (screen_width < 1600) {
        screen_columns = 3;
    } else if (screen_width < 2300) {
        screen_columns = 4;
    } else if (screen_width < 3200) {
        screen_columns = 5;
    }

    int viewport_columns = 6;
    if (viewport_width < 360) {
        viewport_columns = 1;
    } else if (viewport_width < 560) {
        viewport_columns = 2;
    } else if (viewport_width < 800) {
        viewport_columns = 3;
    } else if (viewport_width < 1100) {
        viewport_columns = 4;
    } else if (viewport_width < 1380) {
        viewport_columns = 5;
    }

    return std::min(screen_columns, viewport_columns);
}


}  // namespace shekinah::ui

