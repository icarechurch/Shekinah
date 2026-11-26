#include "shekinah/ui/cue_grid_widget.hpp"

#include "cue_card.hpp"

#include "cue_grid_overlay.hpp"

#include "shekinah/media/slide_renderer.hpp"
#include "shekinah/ui/support/asset_drag.hpp"
#include "shekinah/ui/support/theme.hpp"

#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QEvent>
#include <QFontMetrics>
#include <QGridLayout>
#include <QFrame>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QPushButton>
#include <QResizeEvent>
#include <QScrollArea>
#include <QScreen>
#include <QVBoxLayout>

#include <algorithm>
#include <cmath>

namespace shekinah::ui {
namespace {

constexpr int kGridMarginX = 12;
constexpr int kGridMarginY = 10;
constexpr int kGridSpacing = 12;

}  // namespace

CueGridWidget::CueGridWidget(QWidget* parent) : QWidget(parent), grid_container_(new QWidget(this)) {
    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    empty_scroll_ = new QScrollArea{this};
    empty_scroll_->setObjectName(QStringLiteral("presentation-empty-scroll"));
    empty_scroll_->setFrameShape(QFrame::NoFrame);
    empty_scroll_->setWidgetResizable(true);
    empty_scroll_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    empty_scroll_->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    empty_state_ = new QWidget{empty_scroll_};
    empty_state_->setObjectName(QStringLiteral("presentation-empty-state"));
    auto* empty_layout = new QVBoxLayout{empty_state_};
    empty_layout->setContentsMargins(24, 24, 24, 24);
    empty_layout->setSpacing(8);
    auto* title = new QLabel{QStringLiteral("Presentation"), empty_state_};
    title->setObjectName(QStringLiteral("presentation-title"));
    title->setAlignment(Qt::AlignCenter);
    auto* message = new QLabel{
        QStringLiteral("Add a Song or Sermon to the Service Order, or create content to begin."),
        empty_state_};
    message->setObjectName(QStringLiteral("presentation-empty-message"));
    message->setAlignment(Qt::AlignCenter);
    message->setWordWrap(true);
    empty_layout->addStretch(1);
    empty_layout->addWidget(title);
    empty_layout->addWidget(message);
    const auto add_action = [&](const QString& label, const QString& name, auto handler) {
        auto* button = new QPushButton{label, empty_state_};
        button->setObjectName(name);
        button->setMinimumHeight(34);
        button->setMaximumWidth(320);
        button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        QObject::connect(button, &QPushButton::clicked, this, handler);
        empty_layout->addWidget(button, 0, Qt::AlignHCenter);
    };
    add_action(QStringLiteral("New Song"), QStringLiteral("presentation-new-song"), [this] {
        if (new_song_handler_) {
            new_song_handler_();
        }
    });
    add_action(QStringLiteral("Import Song Lyrics"), QStringLiteral("presentation-import-song"), [this] {
        if (import_song_handler_) {
            import_song_handler_();
        }
    });
    add_action(QStringLiteral("Import Sermon Deck"), QStringLiteral("presentation-import-sermon"), [this] {
        if (import_sermon_handler_) {
            import_sermon_handler_();
        }
    });
    add_action(QStringLiteral("Import Asset"), QStringLiteral("presentation-import-asset"), [this] {
        if (import_asset_handler_) {
            import_asset_handler_();
        }
    });
    empty_layout->addStretch(1);
    empty_scroll_->setWidget(empty_state_);
    outer->addWidget(empty_scroll_);

    grid_scroll_ = new QScrollArea{this};
    grid_scroll_->setObjectName(QStringLiteral("cue-grid-scroll"));
    grid_scroll_->setFrameShape(QFrame::NoFrame);
    grid_scroll_->setWidgetResizable(true);
    grid_scroll_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    grid_scroll_->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    grid_container_->setObjectName(QStringLiteral("cue-grid-container"));
    grid_container_->setAcceptDrops(true);
    grid_container_->installEventFilter(this);
    insert_overlay_ = new InsertOverlay{grid_container_};
    insert_overlay_->hide();
    grid_scroll_->setWidget(grid_container_);
    grid_scroll_->viewport()->setAcceptDrops(true);
    grid_scroll_->viewport()->installEventFilter(this);
    outer->addWidget(grid_scroll_);
    grid_scroll_->setVisible(false);
    setMinimumWidth(220);
    setProperty("cue-insert-index", -1);

    apply_theme();
    QObject::connect(&ThemeManager::instance(), &ThemeManager::theme_changed, this,
                     [this](ThemeId) { apply_theme(); });
}

void CueGridWidget::set_cues(std::vector<CueView> cues) {
    cues_ = std::move(cues);
    const bool empty = cues_.empty();
    empty_scroll_->setVisible(empty);
    grid_scroll_->setVisible(!empty);
    rebuild_cards();
}

void CueGridWidget::set_authoring_handlers(std::function<void()> new_song,
                                           std::function<void()> import_song,
                                           std::function<void()> import_sermon,
                                           std::function<void()> import_asset) {
    new_song_handler_ = std::move(new_song);
    import_song_handler_ = std::move(import_song);
    import_sermon_handler_ = std::move(import_sermon);
    import_asset_handler_ = std::move(import_asset);
}

void CueGridWidget::set_live_cue(std::optional<domain::CueId> cue) {
    live_cue_ = std::move(cue);
    for (auto* card : cards_) {
        card->set_live(live_cue_.has_value() &&
                      card->property("cue-id").toString() ==
                          QString::fromStdString(live_cue_->value));
    }
}

QRect CueGridWidget::visual_rect(const domain::CueId& cue) const {
    const auto card = std::find_if(cards_.begin(), cards_.end(), [&](const auto* candidate) {
        return candidate->objectName() == QStringLiteral("cue-%1").arg(QString::fromStdString(cue.value));
    });
    if (card == cards_.end()) {
        return {};
    }
    return QRect(mapFrom(*card, QPoint{0, 0}), (*card)->size());
}

bool CueGridWidget::eventFilter(QObject* watched, QEvent* event) {
    if (grid_scroll_ != nullptr && watched == grid_scroll_->viewport() &&
        event->type() == QEvent::Resize) {
        relayout_cards(false);
    }
    if (grid_container_ != nullptr && watched == grid_container_ &&
        event->type() == QEvent::Resize && insert_overlay_ != nullptr) {
        insert_overlay_->setGeometry(grid_container_->rect());
    }

    const bool is_container = (grid_container_ != nullptr && watched == grid_container_);
    const bool is_viewport =
        (grid_scroll_ != nullptr && watched == grid_scroll_->viewport());
    if (is_container || is_viewport) {
        const auto to_container_pos = [&](QPoint pos) {
            return is_container ? pos : grid_container_->mapFrom(grid_scroll_->viewport(), pos);
        };
        if (event->type() == QEvent::DragEnter || event->type() == QEvent::DragMove) {
            auto* drag_event = static_cast<QDropEvent*>(event);
            if (asset_id_from_mime_data(drag_event->mimeData()).has_value() && !cards_.empty()) {
                if (const auto target = compute_insert_target(
                        to_container_pos(drag_event->position().toPoint()));
                    target.has_value()) {
                    set_insert_target(*target);
                    drag_event->acceptProposedAction();
                    return true;
                }
            }
            clear_insert_target();
            drag_event->ignore();
            return true;
        }
        if (event->type() == QEvent::DragLeave) {
            clear_insert_target();
            return true;
        }
        if (event->type() == QEvent::Drop) {
            auto* drop_event = static_cast<QDropEvent*>(event);
            const auto asset = asset_id_from_mime_data(drop_event->mimeData());
            const auto target = cards_.empty()
                                    ? std::nullopt
                                    : compute_insert_target(
                                          to_container_pos(drop_event->position().toPoint()));
            clear_insert_target();
            if (!asset.has_value() || !target.has_value()) {
                drop_event->ignore();
                return true;
            }
            handle_asset_insert(target->index, *asset);
            drop_event->acceptProposedAction();
            return true;
        }
    }
    return QWidget::eventFilter(watched, event);
}

void CueGridWidget::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    relayout_cards(false);
}

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
    const int row_count =
        cards_.empty() ? 0 : static_cast<int>((cards_.size() + static_cast<std::size_t>(columns) - 1) / static_cast<std::size_t>(columns));
    layout->setRowStretch(row_count, 1);
    if (insert_overlay_ != nullptr) {
        insert_overlay_->setGeometry(grid_container_->rect());
        insert_overlay_->raise();
    }
}

void CueGridWidget::handle_left_click(const domain::CueId& cue) {
    if (take_live_handler_) {
        take_live_handler_(cue);
    }
}

void CueGridWidget::handle_right_click(const domain::CueId& cue, QPoint global_position) {
    if (context_handler_) {
        context_handler_(cue, global_position);
    }
}

void CueGridWidget::handle_asset_drop(const domain::CueId& cue, const domain::AssetId& asset) {
    if (asset_drop_handler_) {
        asset_drop_handler_(cue, asset);
    }
}

void CueGridWidget::handle_asset_insert(std::size_t index, const domain::AssetId& asset) {
    if (asset_insert_handler_) {
        asset_insert_handler_(index, asset);
    }
}

QRect CueGridWidget::indicator_rect_for_card_edge(std::size_t card_index, bool left_side) const {
    if (card_index >= cards_.size() || grid_container_ == nullptr) {
        return {};
    }
    const auto* card = cards_[card_index];
    int x = 0;
    if (left_side) {
        if (card_index > 0 && cards_[card_index - 1]->y() == card->y()) {
            const int prev_right = cards_[card_index - 1]->geometry().right() + 1;
            x = (prev_right + card->x()) / 2;
        } else {
            x = std::max(3, card->x() / 2);
        }
    } else {
        if (card_index + 1 < cards_.size() && cards_[card_index + 1]->y() == card->y()) {
            const int card_right = card->geometry().right() + 1;
            x = (card_right + cards_[card_index + 1]->x()) / 2;
        } else {
            const int card_right = card->geometry().right() + 1;
            x = std::min(std::max(3, grid_container_->width() - 4),
                         card_right + std::max(3, kGridSpacing / 2));
        }
    }
    return QRect{x - 2, card->y(), 4, card->height()};
}

std::optional<CueGridWidget::InsertTarget> CueGridWidget::compute_insert_target(
    QPoint container_pos) const {
    if (cards_.empty() || grid_container_ == nullptr) {
        return std::nullopt;
    }
    const std::size_t count = cards_.size();
    const std::size_t columns = static_cast<std::size_t>(std::max(1, current_columns_));
    const std::size_t row_count = (count + columns - 1) / columns;

    const auto row_bounds = [&](std::size_t row) -> std::pair<int, int> {
        const std::size_t start = row * columns;
        const std::size_t end = std::min((row + 1) * columns, count);
        int top = cards_[start]->y();
        int bottom = cards_[start]->geometry().bottom();
        for (std::size_t i = start + 1; i < end; ++i) {
            top = std::min(top, cards_[i]->y());
            bottom = std::max(bottom, cards_[i]->geometry().bottom());
        }
        return {top, bottom};
    };

    if (columns == 1) {
        if (container_pos.y() < cards_[0]->y()) {
            if (container_pos.x() < cards_[0]->x()) {
                return InsertTarget{0, indicator_rect_for_card_edge(0, true)};
            }
            const int y = std::max(2, cards_[0]->y() / 2);
            return InsertTarget{0, QRect{cards_[0]->x(), y - 2, cards_[0]->width(), 4}};
        }
        for (std::size_t r = 0; r + 1 < row_count; ++r) {
            const int bottom = cards_[r]->geometry().bottom();
            const int next_top = cards_[r + 1]->y();
            if (container_pos.y() > bottom && container_pos.y() < next_top) {
                const int mid_y = (bottom + 1 + next_top) / 2;
                return InsertTarget{
                    r + 1, QRect{cards_[r]->x(), mid_y - 2, cards_[r]->width(), 4}};
            }
        }
        if (container_pos.y() > cards_.back()->geometry().bottom()) {
            const int y = std::min(std::max(2, grid_container_->height() - 4),
                                   cards_.back()->geometry().bottom() + 1 + kGridSpacing / 2);
            return InsertTarget{
                count, QRect{cards_.back()->x(), y - 2, cards_.back()->width(), 4}};
        }
    }

    const auto last_bounds = row_bounds(row_count - 1);
    if (container_pos.y() > last_bounds.second) {
        return InsertTarget{count, indicator_rect_for_card_edge(count - 1, false)};
    }

    std::size_t target_row = 0;
    if (container_pos.y() > row_bounds(0).second) {
        for (std::size_t r = 0; r < row_count; ++r) {
            const auto bounds = row_bounds(r);
            if (container_pos.y() >= bounds.first && container_pos.y() <= bounds.second) {
                target_row = r;
                break;
            }
            if (r + 1 < row_count) {
                const auto next_bounds = row_bounds(r + 1);
                if (container_pos.y() > bounds.second && container_pos.y() < next_bounds.first) {
                    const int mid_y = (bounds.second + 1 + next_bounds.first) / 2;
                    target_row = container_pos.y() <= mid_y ? r : (r + 1);
                    break;
                }
            }
        }
    }

    const std::size_t row_start = target_row * columns;
    const std::size_t row_end = std::min((target_row + 1) * columns, count);
    if (container_pos.x() < cards_[row_start]->geometry().center().x()) {
        return InsertTarget{row_start, indicator_rect_for_card_edge(row_start, true)};
    }
    for (std::size_t idx = row_start; idx + 1 < row_end; ++idx) {
        if (container_pos.x() < cards_[idx + 1]->geometry().center().x()) {
            return InsertTarget{idx + 1, indicator_rect_for_card_edge(idx, false)};
        }
    }
    return InsertTarget{row_end, indicator_rect_for_card_edge(row_end - 1, false)};
}

void CueGridWidget::set_insert_target(InsertTarget target) {
    insert_target_ = target;
    setProperty("cue-insert-index", static_cast<int>(target.index));
    if (insert_overlay_ != nullptr) {
        insert_overlay_->set_indicator_rect(target.indicator_rect);
    }
}

void CueGridWidget::set_insert_target_for_card(
    std::size_t card_index, std::size_t insert_index, bool left_side) {
    set_insert_target(InsertTarget{
        insert_index,
        indicator_rect_for_card_edge(card_index, left_side),
    });
}

void CueGridWidget::clear_insert_target() {
    insert_target_.reset();
    setProperty("cue-insert-index", -1);
    if (insert_overlay_ != nullptr) {
        insert_overlay_->set_indicator_rect({});
    }
}

}  // namespace shekinah::ui
