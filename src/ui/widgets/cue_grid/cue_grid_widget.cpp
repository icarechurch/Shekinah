#include "shekinah/ui/cue_grid_widget.hpp"

#include <QGridLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QPushButton>
#include <QResizeEvent>
#include <QScreen>
#include <QVBoxLayout>

#include <algorithm>
#include <cmath>

namespace shekinah::ui {

class CueGridWidget::CueCard final : public QWidget {
public:
    CueCard(const CueView& cue, CueGridWidget& owner) : QWidget(&owner), cue_(cue), owner_(owner) {
        setObjectName(QStringLiteral("cue-%1").arg(QString::fromStdString(cue.id.value)));
        setMinimumSize(120, 100);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        setCursor(Qt::PointingHandCursor);
        setAttribute(Qt::WA_Hover, true);
        setProperty("cue-section", cue.section);
        setProperty("cue-summary", cue.summary);
        setProperty("cue-live", false);
        setToolTip(cue.summary);
    }

    int heightForWidth(int width) const override {
        return std::max(100, static_cast<int>(std::lround(static_cast<double>(width) * 9.0 / 16.0)) + 32);
    }

    bool hasHeightForWidth() const override { return true; }

    QRect preview_rect() const {
        constexpr int border = 2;
        constexpr int footer = 32;
        return rect().adjusted(border, border, -border, -footer - border);
    }

    void set_live(bool live) {
        setProperty("cue-live", live);
        update();
    }

protected:
    void mousePressEvent(QMouseEvent* event) override {
        if (event->button() == Qt::LeftButton) {
            owner_.handle_left_click(cue_.id);
        } else if (event->button() == Qt::RightButton) {
            owner_.handle_right_click(cue_.id, mapToGlobal(event->position().toPoint()));
        }
        event->accept();
    }

    void paintEvent(QPaintEvent*) override {
        QPainter painter{this};
        painter.fillRect(rect(), QColor{QStringLiteral("#121212")});
        const auto preview = preview_rect();
        constexpr int tile_size = 16;
        for (int y = preview.top(); y < preview.bottom(); y += tile_size) {
            for (int x = preview.left(); x < preview.right(); x += tile_size) {
                const auto tile = ((x - preview.left()) / tile_size +
                                   (y - preview.top()) / tile_size) % 2;
                painter.fillRect(
                    QRect{x, y, tile_size, tile_size},
                    tile == 0 ? QColor{QStringLiteral("#111111")}
                              : QColor{QStringLiteral("#282828")});
            }
        }
    }

private:
    CueView cue_;
    CueGridWidget& owner_;
};

CueGridWidget::CueGridWidget(QWidget* parent) : QWidget(parent), grid_container_(new QWidget(this)) {
    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    empty_state_ = new QWidget{this};
    empty_state_->setObjectName(QStringLiteral("presentation-empty-state"));
    auto* empty_layout = new QVBoxLayout{empty_state_};
    empty_layout->setContentsMargins(32, 32, 32, 32);
    auto* title = new QLabel{QStringLiteral("Presentation"), empty_state_};
    title->setObjectName(QStringLiteral("presentation-title"));
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet(QStringLiteral("font-size: 24px; font-weight: 600;"));
    auto* message = new QLabel{
        QStringLiteral("Add a Song or Sermon to the Service Order, or create content to begin."),
        empty_state_};
    message->setAlignment(Qt::AlignCenter);
    message->setWordWrap(true);
    empty_layout->addStretch(1);
    empty_layout->addWidget(title);
    empty_layout->addWidget(message);
    const auto add_action = [&](const QString& label, const QString& name, auto handler) {
        auto* button = new QPushButton{label, empty_state_};
        button->setObjectName(name);
        button->setMinimumHeight(38);
        QObject::connect(button, &QPushButton::clicked, this, handler);
        empty_layout->addWidget(button);
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
    outer->addWidget(empty_state_);
    outer->addWidget(grid_container_);
    grid_container_->setVisible(false);
    setMinimumWidth(240);
}

int CueGridWidget::column_count_for_geometry(int viewport_width, int screen_width) {
    if (viewport_width < 320) {
        return 1;
    }
    if (viewport_width < 520) {
        return 2;
    }
    if (screen_width < 1600) {
        return 3;
    }
    if (screen_width < 2300) {
        return 4;
    }
    if (screen_width < 3200) {
        return 5;
    }
    return 6;
}

void CueGridWidget::set_cues(std::vector<CueView> cues) {
    cues_ = std::move(cues);
    const bool empty = cues_.empty();
    empty_state_->setVisible(empty);
    grid_container_->setVisible(!empty);
    rebuild_layout();
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

void CueGridWidget::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    rebuild_layout();
}

void CueGridWidget::rebuild_layout() {
    if (grid_container_ == nullptr) {
        return;
    }
    if (auto* old_layout = grid_container_->layout(); old_layout != nullptr) {
        while (auto* item = old_layout->takeAt(0)) {
            if (auto* widget = item->widget(); widget != nullptr) {
                widget->deleteLater();
            }
            delete item;
        }
        delete old_layout;
    }
    cards_.clear();

    auto* layout = new QGridLayout(grid_container_);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(8);
    const int screen_width = screen() == nullptr ? 1920 : screen()->size().width();
    const int columns = column_count_for_geometry(width(), screen_width);
    for (std::size_t index = 0; index != cues_.size(); ++index) {
        auto* card = new CueCard{cues_[index], *this};
        card->setProperty("cue-id", QString::fromStdString(cues_[index].id.value));
        card->set_live(live_cue_.has_value() && cues_[index].id == *live_cue_);
        cards_.push_back(card);
        const int row = static_cast<int>(index / static_cast<std::size_t>(columns));
        const int column = static_cast<int>(index % static_cast<std::size_t>(columns));
        layout->addWidget(card, row, column);
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

}  // namespace shekinah::ui
