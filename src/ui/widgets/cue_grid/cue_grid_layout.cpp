#include "shekinah/ui/cue_grid_widget.hpp"

#include "cue_card.hpp"
#include "cue_grid_overlay.hpp"

#include <QGridLayout>
#include <QResizeEvent>
#include <QScreen>
#include <QScrollArea>

#include <algorithm>

namespace shekinah::ui {
namespace {

constexpr int kGridMarginX = 12;
constexpr int kGridMarginY = 10;
constexpr int kGridSpacing = 12;

}  // namespace

void CueGridWidget::rebuild_cards() {
    if (grid_container_ == nullptr) {
        return;
    }
    clear_insert_target();
    if (auto* old_layout = grid_container_->layout(); old_layout != nullptr) {
        while (auto* item = old_layout->takeAt(0)) {
            delete item;
        }
        delete old_layout;
    }
    for (auto* card : cards_) {
        card->deleteLater();
    }
    cards_.clear();
    cards_.reserve(cues_.size());

    for (std::size_t index = 0; index != cues_.size(); ++index) {
        const auto& cue = cues_[index];
        auto* card = new CueCard{cue, index, *this};
        card->setProperty("cue-id", QString::fromStdString(cue.id.value));
        card->set_live(live_cue_.has_value() && cue.id == *live_cue_);
        cards_.push_back(card);
    }
    relayout_cards(true);
}

void CueGridWidget::relayout_cards(bool force) {
    if (grid_container_ == nullptr) {
        return;
    }
    const int viewport_width =
        (grid_scroll_ != nullptr && grid_scroll_->viewport()->width() > 0)
            ? grid_scroll_->viewport()->width()
            : width();
    const int screen_width = screen() == nullptr ? 1920 : screen()->size().width();
    const int columns = column_count_for_geometry(viewport_width, screen_width);
    if (!force && columns == current_columns_ && grid_container_->layout() != nullptr) {
        return;
    }
    current_columns_ = columns;

    if (auto* old_layout = grid_container_->layout(); old_layout != nullptr) {
        while (auto* item = old_layout->takeAt(0)) {
            delete item;
        }
        delete old_layout;
    }

    auto* layout = new QGridLayout(grid_container_);
    layout->setContentsMargins(kGridMarginX, kGridMarginY, kGridMarginX, kGridMarginY);
    layout->setSpacing(kGridSpacing);
    for (int col = 0; col < columns; ++col) {
        layout->setColumnStretch(col, 1);
    }
    for (std::size_t index = 0; index != cards_.size(); ++index) {
        const int row = static_cast<int>(index / static_cast<std::size_t>(columns));
        const int column = static_cast<int>(index % static_cast<std::size_t>(columns));
        layout->addWidget(cards_[index], row, column);
    }
    const int row_count = cards_.empty()
                              ? 0
                              : static_cast<int>((cards_.size() + static_cast<std::size_t>(columns) - 1) /
                                                 static_cast<std::size_t>(columns));
    layout->setRowStretch(row_count, 1);
    if (insert_overlay_ != nullptr) {
        insert_overlay_->setGeometry(grid_container_->rect());
        insert_overlay_->raise();
    }
}

}  // namespace shekinah::ui
