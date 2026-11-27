#include "shekinah/ui/library_dock_widget.hpp"

#include <QLabel>
#include <QStackedWidget>
#include <QVBoxLayout>

namespace shekinah::ui {

void LibraryDockWidget::setup_content_empty_overlay() {
    empty_overlay_ = new QWidget{stack_};
    empty_overlay_->setObjectName(QStringLiteral("library-empty-state"));
    empty_overlay_->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    auto* empty_layout = new QVBoxLayout(empty_overlay_);
    empty_layout->setContentsMargins(24, 12, 24, 12);
    empty_layout->setSpacing(4);
    empty_layout->addStretch(1);
    empty_icon_label_ = new QLabel{empty_overlay_};
    empty_icon_label_->setAlignment(Qt::AlignCenter);
    empty_layout->addWidget(empty_icon_label_);
    empty_title_label_ = new QLabel{empty_overlay_};
    empty_title_label_->setObjectName(QStringLiteral("library-empty-title"));
    empty_title_label_->setAlignment(Qt::AlignCenter);
    empty_layout->addWidget(empty_title_label_);
    empty_hint_label_ = new QLabel{empty_overlay_};
    empty_hint_label_->setObjectName(QStringLiteral("library-empty-hint"));
    empty_hint_label_->setAlignment(Qt::AlignCenter);
    empty_hint_label_->setWordWrap(true);
    empty_layout->addWidget(empty_hint_label_);
    empty_layout->addStretch(1);
}

}  // namespace shekinah::ui
