#include "shekinah/ui/cue_grid_widget.hpp"

#include "cue_card.hpp"
#include "cue_grid_overlay.hpp"

#include "shekinah/ui/support/theme.hpp"

#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>

namespace shekinah::ui {

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

}  // namespace shekinah::ui
