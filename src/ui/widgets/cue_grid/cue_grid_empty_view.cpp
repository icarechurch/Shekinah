#include "shekinah/ui/cue_grid_widget.hpp"

#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>

namespace shekinah::ui {

void CueGridWidget::setup_empty_view(QVBoxLayout* outer_layout) {
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
    outer_layout->addWidget(empty_scroll_);
}

}  // namespace shekinah::ui
