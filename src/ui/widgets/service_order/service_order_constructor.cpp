#include "shekinah/ui/service_order_widget.hpp"

#include "service_order_primitives.hpp"
#include "service_order_list.hpp"

#include "shekinah/ui/support/asset_drag.hpp"
#include "shekinah/ui/support/content_drag.hpp"
#include "shekinah/ui/support/elided_label.hpp"
#include "shekinah/ui/support/theme.hpp"

#include <QAbstractItemView>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>

namespace shekinah::ui {
ServiceOrderWidget::ServiceOrderWidget(QWidget* parent) : QWidget(parent) {
    setObjectName(QStringLiteral("service-order"));

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // 1. Header Bar
    auto* header_bar = new QWidget{this};
    header_bar->setObjectName(QStringLiteral("service-order-header"));
    header_bar->setFixedHeight(26);
    auto* header_layout = new QHBoxLayout(header_bar);
    header_layout->setContentsMargins(10, 0, 6, 0);
    header_layout->setSpacing(6);

    auto* title_label = new ElidedLabel{QStringLiteral("SERVICE ORDER"), header_bar};
    title_label->setObjectName(QStringLiteral("service-order-title"));
    header_layout->addWidget(title_label);
    header_layout->addStretch(1);

    count_badge_ = new QLabel{QStringLiteral("0 ITEMS"), header_bar};
    count_badge_->setObjectName(QStringLiteral("service-order-count"));
    count_badge_->setFixedHeight(18);
    count_badge_->setAlignment(Qt::AlignCenter);
    header_layout->addWidget(count_badge_, 0, Qt::AlignVCenter);

    add_button_ = new QPushButton{header_bar};
    add_button_->setObjectName(QStringLiteral("service-order-add"));
    add_button_->setFixedSize(20, 20);
    add_button_->setCursor(Qt::PointingHandCursor);
    add_button_->setToolTip(QStringLiteral("New or Import Presentation Item"));
    add_button_->setIconSize(QSize{13, 13});
    add_button_->setEnabled(false);
    QObject::connect(add_button_, &QPushButton::clicked, this, [this] { open_add_menu(); });
    header_layout->addWidget(add_button_);
    layout->addWidget(header_bar);

    // 2. Service Order List + Transparent Empty-State Overlay
    auto* list_widget = new service_order_internal::ServiceOrderListWidget{this};
    list_ = list_widget;
    list_->setObjectName(QStringLiteral("service-order-list"));
    list_->setFrameShape(QFrame::NoFrame);
    list_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    list_->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    list_->setSelectionMode(QAbstractItemView::SingleSelection);
    list_->setUniformItemSizes(true);
    list_->setAcceptDrops(true);
    list_->viewport()->setAcceptDrops(true);
    list_->setContextMenuPolicy(Qt::CustomContextMenu);

    empty_overlay_ = new QWidget{list_->viewport()};
    empty_overlay_->setObjectName(QStringLiteral("service-order-empty-state"));
    empty_overlay_->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    auto* empty_layout = new QVBoxLayout(empty_overlay_);
    empty_layout->setContentsMargins(18, 24, 18, 24);
    empty_layout->setSpacing(6);
    empty_layout->addStretch(1);

    empty_icon_label_ = new QLabel{empty_overlay_};
    empty_icon_label_->setAlignment(Qt::AlignCenter);
    empty_layout->addWidget(empty_icon_label_);

    empty_title_label_ = new QLabel{QStringLiteral("Service Order Empty"), empty_overlay_};
    empty_title_label_->setObjectName(QStringLiteral("service-order-empty-title"));
    empty_title_label_->setAlignment(Qt::AlignCenter);
    empty_layout->addWidget(empty_title_label_);

    empty_hint_label_ = new QLabel{
        QStringLiteral("Drag a Song or Sermon here, or double-click an item in the Library below."),
        empty_overlay_};
    empty_hint_label_->setObjectName(QStringLiteral("service-order-empty-hint"));
    empty_hint_label_->setAlignment(Qt::AlignCenter);
    empty_hint_label_->setWordWrap(true);
    empty_layout->addWidget(empty_hint_label_);
    empty_layout->addStretch(1);

    list_widget->set_viewport_resized_handler([this] {
        if (empty_overlay_ != nullptr && list_ != nullptr) {
            empty_overlay_->setGeometry(list_->viewport()->rect());
        }
    });
    list_widget->set_asset_drop_handler([this](const QMimeData* data, QPoint position) {
        const auto asset = asset_id_from_mime_data(data);
        auto* item = list_->itemAt(position);
        const auto row = item == nullptr ? -1 : list_->row(item);
        if (!asset.has_value() || row < 0 || row >= static_cast<int>(items_.size()) ||
            !asset_drop_handler_) {
            return false;
        }
        asset_drop_handler_(items_[static_cast<std::size_t>(row)].item.content, *asset);
        return true;
    });
    list_widget->set_content_drop_handler([this](const QMimeData* data) {
        const auto content = content_ref_from_mime_data(data);
        if (!content.has_value() || !content_drop_handler_) {
            return false;
        }
        content_drop_handler_(*content);
        return true;
    });
    list_widget->set_key_action_handlers(
        [this](int delta) { move_selected(delta); },
        [this] { remove_selected(); },
        [this] { edit_selected(); });

    QObject::connect(list_, &QListWidget::currentRowChanged, this, [this](int row) {
        update_controls_state();
        if (row >= 0 && row < static_cast<int>(items_.size()) && selection_handler_) {
            selection_handler_(items_[static_cast<std::size_t>(row)].item.id);
        }
    });
    QObject::connect(list_, &QListWidget::customContextMenuRequested, this,
                     [this](const QPoint& pos) { open_context_menu(pos); });
    QObject::connect(list_, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem*) {
        edit_selected();
    });
    layout->addWidget(list_, 1);

    // 3. Compact Reorder & Action Toolbar
    auto* controls_bar = new QWidget{this};
    controls_bar->setObjectName(QStringLiteral("service-order-controls"));
    controls_bar->setFixedHeight(32);
    auto* controls = new QHBoxLayout(controls_bar);
    controls->setContentsMargins(6, 4, 6, 4);
    controls->setSpacing(4);

    move_up_button_ = new QPushButton{controls_bar};
    move_up_button_->setObjectName(QStringLiteral("service-order-move-up"));
    move_up_button_->setProperty("orderTool", true);
    move_up_button_->setFixedSize(28, 24);
    move_up_button_->setCursor(Qt::PointingHandCursor);
    move_up_button_->setToolTip(QStringLiteral("Move Item Up (Alt+Up)"));
    move_up_button_->setIconSize(QSize{13, 13});

    move_down_button_ = new QPushButton{controls_bar};
    move_down_button_->setObjectName(QStringLiteral("service-order-move-down"));
    move_down_button_->setProperty("orderTool", true);
    move_down_button_->setFixedSize(28, 24);
    move_down_button_->setCursor(Qt::PointingHandCursor);
    move_down_button_->setToolTip(QStringLiteral("Move Item Down (Alt+Down)"));
    move_down_button_->setIconSize(QSize{13, 13});

    edit_button_ = new QPushButton{controls_bar};
    edit_button_->setObjectName(QStringLiteral("service-order-edit"));
    edit_button_->setProperty("orderTool", true);
    edit_button_->setFixedSize(28, 24);
    edit_button_->setCursor(Qt::PointingHandCursor);
    edit_button_->setToolTip(QStringLiteral("Edit Selected Song or Sermon (Ctrl+Enter)"));
    edit_button_->setIconSize(QSize{13, 13});

    remove_button_ = new QPushButton{QStringLiteral("Remove"), controls_bar};
    remove_button_->setObjectName(QStringLiteral("service-order-remove"));
    remove_button_->setProperty("orderTool", true);
    remove_button_->setFixedHeight(24);
    remove_button_->setCursor(Qt::PointingHandCursor);
    remove_button_->setToolTip(QStringLiteral("Remove Selected Item from Service Order (Del)"));
    remove_button_->setIconSize(QSize{12, 12});

    QObject::connect(move_up_button_, &QPushButton::clicked, this, [this] { move_selected(-1); });
    QObject::connect(move_down_button_, &QPushButton::clicked, this, [this] { move_selected(1); });
    QObject::connect(edit_button_, &QPushButton::clicked, this, [this] { edit_selected(); });
    QObject::connect(remove_button_, &QPushButton::clicked, this, [this] { remove_selected(); });

    controls->addWidget(move_up_button_);
    controls->addWidget(move_down_button_);
    controls->addWidget(edit_button_);
    controls->addStretch(1);
    controls->addWidget(remove_button_);
    layout->addWidget(controls_bar);

    // 4. Quick Filter Bar (bottom)
    auto* filter_bar = new QWidget{this};
    filter_bar->setObjectName(QStringLiteral("service-order-filter-bar"));
    filter_bar->setFixedHeight(32);
    auto* filter_layout = new QHBoxLayout(filter_bar);
    filter_layout->setContentsMargins(6, 4, 6, 4);
    filter_layout->setSpacing(0);

    filter_input_ = new QLineEdit{filter_bar};
    filter_input_->setObjectName(QStringLiteral("service-order-filter"));
    filter_input_->setPlaceholderText(QStringLiteral("Filter service..."));
    filter_input_->setClearButtonEnabled(true);
    QObject::connect(filter_input_, &QLineEdit::textChanged, this, [this] { apply_filter(); });
    filter_layout->addWidget(filter_input_);
    layout->addWidget(filter_bar);

    apply_theme();
    QObject::connect(&ThemeManager::instance(), &ThemeManager::theme_changed, this,
                     [this](ThemeId) { apply_theme(); });

    update_empty_state();
    update_controls_state();
}

}  // namespace shekinah::ui
