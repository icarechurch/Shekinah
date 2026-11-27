#include "shekinah/ui/cue_grid_widget.hpp"

#include "cue_card.hpp"
#include "shekinah/ui/support/theme.hpp"

#include <QScrollArea>
#include <QVBoxLayout>

namespace shekinah::ui {

CueGridWidget::CueGridWidget(QWidget* parent) : QWidget(parent), grid_container_(new QWidget(this)) {
    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    setup_empty_view(outer);
    setup_grid_view(outer);
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
