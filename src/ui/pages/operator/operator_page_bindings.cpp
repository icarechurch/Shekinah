#include "shekinah/ui/operator_page.hpp"

#include "shekinah/ui/cue_grid_widget.hpp"
#include "shekinah/ui/program_status_widget.hpp"
#include "shekinah/ui/service_order_widget.hpp"

#include <QLabel>

namespace shekinah::ui {

void OperatorPage::set_active_presentation_title(const QString& title) {
    const QString trimmed = title.trimmed();
    if (trimmed.isEmpty()) {
        presentation_item_label_->clear();
        presentation_item_label_->setVisible(false);
    } else {
        presentation_item_label_->setText(QStringLiteral("·  %1").arg(trimmed));
        presentation_item_label_->setVisible(true);
    }
}

void OperatorPage::set_authoring_handlers(std::function<void()> new_song,
                                          std::function<void()> import_song,
                                          std::function<void()> import_sermon,
                                          std::function<void()> import_asset) {
    new_song_handler_ = new_song;
    import_song_handler_ = import_song;
    import_sermon_handler_ = import_sermon;
    service_order_->set_authoring_handlers(
        new_song_handler_, import_song_handler_, new_sermon_handler_, import_sermon_handler_);
    cue_grid_->set_authoring_handlers(
        std::move(new_song), std::move(import_song), std::move(import_sermon), std::move(import_asset));
}

void OperatorPage::set_take_live_handler(std::function<void(domain::CueId)> handler) {
    cue_grid_->set_take_live_handler(handler);
    program_status_->set_take_cue_handler(std::move(handler));
}

void OperatorPage::set_background_handler(std::function<void(domain::CueId)> handler) {
    background_handler_ = std::move(handler);
}

}  // namespace shekinah::ui
