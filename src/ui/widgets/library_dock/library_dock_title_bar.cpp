#include "shekinah/ui/library_dock_widget.hpp"

#include "shekinah/ui/support/elided_label.hpp"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

namespace shekinah::ui {

void LibraryDockWidget::setup_title_bar() {
    auto* title_bar = new QWidget{this};
    title_bar->setObjectName(QStringLiteral("library-dock-header"));
    title_bar->setFixedHeight(26);
    auto* title_layout = new QHBoxLayout(title_bar);
    title_layout->setContentsMargins(10, 0, 6, 0);
    title_layout->setSpacing(6);

    auto* title_label = new QLabel{QStringLiteral("LIBRARY"), title_bar};
    title_label->setObjectName(QStringLiteral("library-dock-title"));
    title_layout->addWidget(title_label);

    header_count_badge_ = new QLabel{QStringLiteral("0 ITEMS"), title_bar};
    header_count_badge_->setObjectName(QStringLiteral("library-total-count"));
    header_count_badge_->setFixedHeight(18);
    header_count_badge_->setAlignment(Qt::AlignCenter);
    title_layout->addWidget(header_count_badge_, 0, Qt::AlignVCenter);

    auto* hint_label = new ElidedLabel{
        QStringLiteral("Drag Assets to Slides/Orders  Ãƒâ€šÃ‚Â·  Double-click Song/Sermon to add"),
        title_bar};
    hint_label->setObjectName(QStringLiteral("library-dock-hint"));
    hint_label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    title_layout->addWidget(hint_label, 1);

    float_toggle_button_ = new QPushButton{title_bar};
    float_toggle_button_->setObjectName(QStringLiteral("library-float-toggle"));
    float_toggle_button_->setFixedSize(20, 20);
    float_toggle_button_->setCursor(Qt::PointingHandCursor);
    float_toggle_button_->setToolTip(QStringLiteral("Float or Dock Library"));
    float_toggle_button_->setIconSize(QSize{12, 12});
    QObject::connect(float_toggle_button_, &QPushButton::clicked, this, [this] {
        setFloating(!isFloating());
    });
    title_layout->addWidget(float_toggle_button_);
    setTitleBarWidget(title_bar);
}

}  // namespace shekinah::ui
