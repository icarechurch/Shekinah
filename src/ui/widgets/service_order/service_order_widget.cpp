#include "shekinah/ui/service_order_widget.hpp"
#include "shekinah/ui/support/asset_drag.hpp"
#include "shekinah/ui/support/content_drag.hpp"

#include <QAbstractItemView>
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QFrame>
#include <QHBoxLayout>
#include <QIcon>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMenu>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QPushButton>
#include <QResizeEvent>
#include <QStyledItemDelegate>
#include <QVBoxLayout>

#include <algorithm>

namespace shekinah::ui {
namespace {

constexpr int kKindRole = Qt::UserRole + 1;
constexpr int kSlideCountRole = Qt::UserRole + 2;
constexpr int kHasBgRole = Qt::UserRole + 3;
constexpr int kIsLiveRole = Qt::UserRole + 4;
constexpr int kOrdinalRole = Qt::UserRole + 5;
constexpr int kDropTargetRole = Qt::UserRole + 6;
constexpr int kRowHeight = 46;

enum class OrderGlyph {
    plus,
    chevron_up,
    chevron_down,
    edit,
    trash,
    song,
    sermon,
    empty_stack,
};

QIcon make_order_icon(OrderGlyph glyph, const QColor& color, int size = 14) {
    QPixmap pixmap{size * 2, size * 2};
    pixmap.fill(Qt::transparent);
    pixmap.setDevicePixelRatio(2.0);
    QPainter p{&pixmap};
    p.setRenderHint(QPainter::Antialiasing, true);
    QPen pen{color, 1.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin};
    p.setPen(pen);
    const qreal s = static_cast<qreal>(size);

    switch (glyph) {
    case OrderGlyph::plus:
        p.drawLine(QPointF{s * 0.50, s * 0.22}, QPointF{s * 0.50, s * 0.78});
        p.drawLine(QPointF{s * 0.22, s * 0.50}, QPointF{s * 0.78, s * 0.50});
        break;
    case OrderGlyph::chevron_up:
        p.drawLine(QPointF{s * 0.24, s * 0.62}, QPointF{s * 0.50, s * 0.34});
        p.drawLine(QPointF{s * 0.50, s * 0.34}, QPointF{s * 0.76, s * 0.62});
        break;
    case OrderGlyph::chevron_down:
        p.drawLine(QPointF{s * 0.24, s * 0.38}, QPointF{s * 0.50, s * 0.66});
        p.drawLine(QPointF{s * 0.50, s * 0.66}, QPointF{s * 0.76, s * 0.38});
        break;
    case OrderGlyph::edit: {
        QPainterPath path;
        path.moveTo(s * 0.22, s * 0.78);
        path.lineTo(s * 0.26, s * 0.60);
        path.lineTo(s * 0.68, s * 0.18);
        path.lineTo(s * 0.82, s * 0.32);
        path.lineTo(s * 0.40, s * 0.74);
        path.closeSubpath();
        p.drawPath(path);
        p.drawLine(QPointF{s * 0.58, s * 0.28}, QPointF{s * 0.72, s * 0.42});
        break;
    }
    case OrderGlyph::trash:
        p.drawLine(QPointF{s * 0.20, s * 0.28}, QPointF{s * 0.80, s * 0.28});
        p.drawLine(QPointF{s * 0.38, s * 0.18}, QPointF{s * 0.62, s * 0.18});
        p.drawRoundedRect(QRectF{s * 0.28, s * 0.28, s * 0.44, s * 0.54}, 1.5, 1.5);
        p.drawLine(QPointF{s * 0.42, s * 0.42}, QPointF{s * 0.42, s * 0.68});
        p.drawLine(QPointF{s * 0.58, s * 0.42}, QPointF{s * 0.58, s * 0.68});
        break;
    case OrderGlyph::song:
        p.setBrush(color);
        p.drawEllipse(QPointF{s * 0.32, s * 0.72}, s * 0.12, s * 0.09);
        p.drawEllipse(QPointF{s * 0.68, s * 0.64}, s * 0.12, s * 0.09);
        p.setBrush(Qt::NoBrush);
        p.drawLine(QPointF{s * 0.43, s * 0.70}, QPointF{s * 0.43, s * 0.24});
        p.drawLine(QPointF{s * 0.79, s * 0.62}, QPointF{s * 0.79, s * 0.18});
        p.drawLine(QPointF{s * 0.43, s * 0.24}, QPointF{s * 0.79, s * 0.18});
        break;
    case OrderGlyph::sermon:
        p.drawRoundedRect(QRectF{s * 0.18, s * 0.20, s * 0.64, s * 0.46}, 2.0, 2.0);
        p.drawLine(QPointF{s * 0.32, s * 0.36}, QPointF{s * 0.68, s * 0.36});
        p.drawLine(QPointF{s * 0.32, s * 0.50}, QPointF{s * 0.58, s * 0.50});
        p.drawLine(QPointF{s * 0.50, s * 0.66}, QPointF{s * 0.50, s * 0.82});
        p.drawLine(QPointF{s * 0.34, s * 0.82}, QPointF{s * 0.66, s * 0.82});
        break;
    case OrderGlyph::empty_stack:
        p.drawRoundedRect(QRectF{s * 0.18, s * 0.22, s * 0.64, s * 0.24}, 2.5, 2.5);
        p.drawRoundedRect(QRectF{s * 0.18, s * 0.54, s * 0.64, s * 0.24}, 2.5, 2.5);
        break;
    }
    return QIcon{pixmap};
}

void draw_badge_glyph(QPainter& p, const QRectF& rect, domain::ContentKind kind, const QColor& color) {
    p.save();
    p.setRenderHint(QPainter::Antialiasing, true);
    QPen pen{color, 1.4, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin};
    p.setPen(pen);
    const qreal x = rect.left();
    const qreal y = rect.top();
    const qreal w = rect.width();
    const qreal h = rect.height();

    if (kind == domain::ContentKind::song) {
        p.setBrush(color);
        p.drawEllipse(QPointF{x + w * 0.32, y + h * 0.72}, w * 0.11, h * 0.08);
        p.drawEllipse(QPointF{x + w * 0.67, y + h * 0.64}, w * 0.11, h * 0.08);
        p.setBrush(Qt::NoBrush);
        p.drawLine(QPointF{x + w * 0.42, y + h * 0.70}, QPointF{x + w * 0.42, y + h * 0.26});
        p.drawLine(QPointF{x + w * 0.77, y + h * 0.62}, QPointF{x + w * 0.77, y + h * 0.20});
        p.drawLine(QPointF{x + w * 0.42, y + h * 0.26}, QPointF{x + w * 0.77, y + h * 0.20});
    } else {
        p.drawRoundedRect(QRectF{x + w * 0.20, y + h * 0.22, w * 0.60, h * 0.42}, 1.5, 1.5);
        p.drawLine(QPointF{x + w * 0.33, y + h * 0.37}, QPointF{x + w * 0.67, y + h * 0.37});
        p.drawLine(QPointF{x + w * 0.33, y + h * 0.50}, QPointF{x + w * 0.56, y + h * 0.50});
        p.drawLine(QPointF{x + w * 0.50, y + h * 0.64}, QPointF{x + w * 0.50, y + h * 0.78});
        p.drawLine(QPointF{x + w * 0.36, y + h * 0.78}, QPointF{x + w * 0.64, y + h * 0.78});
    }
    p.restore();
}

class ServiceOrderItemDelegate final : public QStyledItemDelegate {
public:
    using QStyledItemDelegate::QStyledItemDelegate;

    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex&) const override {
        return {option.rect.width(), kRowHeight};
    }

    void paint(QPainter* painter,
               const QStyleOptionViewItem& option,
               const QModelIndex& index) const override {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing, true);

        const QRect bounds = option.rect;
        const bool selected = option.state.testFlag(QStyle::State_Selected);
        const bool hovered = option.state.testFlag(QStyle::State_MouseOver);
        const bool is_live = index.data(kIsLiveRole).toBool();
        const bool drop_target = index.data(kDropTargetRole).toBool();
        const auto kind = static_cast<domain::ContentKind>(index.data(kKindRole).toInt());
        const int slide_count = index.data(kSlideCountRole).toInt();
        const bool has_bg = index.data(kHasBgRole).toBool();
        const int ordinal = index.data(kOrdinalRole).toInt();
        const QString title = index.data(Qt::DisplayRole).toString();

        // Row background
        QColor bg_color{QStringLiteral("#1b1c21")};
        if (drop_target) {
            bg_color = QColor{QStringLiteral("#183244")};
        } else if (selected) {
            bg_color = QColor{QStringLiteral("#1d324d")};
        } else if (hovered) {
            bg_color = QColor{QStringLiteral("#23252d")};
        }
        painter->fillRect(bounds, bg_color);

        // Bottom hairline divider
        painter->setPen(QColor{QStringLiteral("#24262e")});
        painter->drawLine(bounds.bottomLeft(), bounds.bottomRight());

        // Left status / selection accent bar
        if (is_live) {
            painter->fillRect(QRect{bounds.left(), bounds.top() + 3, 3, bounds.height() - 6},
                              QColor{QStringLiteral("#22e58a")});
        } else if (selected) {
            painter->fillRect(QRect{bounds.left(), bounds.top() + 3, 3, bounds.height() - 6},
                              QColor{QStringLiteral("#0a84ff")});
        }

        // Drop target highlight outline
        if (drop_target) {
            painter->setPen(QPen{QColor{QStringLiteral("#65d6ff")}, 1.5});
            painter->drawRoundedRect(bounds.adjusted(2, 2, -2, -2), 4.0, 4.0);
        }

        // Content type icon badge (left)
        const QRectF badge_rect{
            static_cast<qreal>(bounds.left() + 10),
            static_cast<qreal>(bounds.top() + (bounds.height() - 26) / 2),
            26.0,
            26.0,
        };
        const bool is_song = (kind == domain::ContentKind::song);
        const QColor badge_fill = is_song ? QColor{14, 165, 233, 34} : QColor{245, 158, 11, 34};
        const QColor badge_fg = is_song ? QColor{QStringLiteral("#38bdf8")}
                                        : QColor{QStringLiteral("#fbbf24")};
        painter->setPen(QPen{is_song ? QColor{56, 189, 248, 70} : QColor{251, 191, 36, 70}, 1.0});
        painter->setBrush(badge_fill);
        painter->drawRoundedRect(badge_rect, 5.0, 5.0);
        draw_badge_glyph(*painter, badge_rect, kind, badge_fg);

        // Right-side pill ("LIVE" or ordinal "#01")
        const int right_padding = 8;
        int right_occupied = right_padding;
        if (is_live) {
            const QRect live_pill{bounds.right() - 40, bounds.top() + (bounds.height() - 16) / 2, 32, 16};
            painter->setPen(QPen{QColor{34, 229, 138, 120}, 1.0});
            painter->setBrush(QColor{34, 229, 138, 42});
            painter->drawRoundedRect(live_pill, 3.0, 3.0);
            QFont pill_font = option.font;
            pill_font.setPixelSize(9);
            pill_font.setBold(true);
            painter->setFont(pill_font);
            painter->setPen(QColor{QStringLiteral("#22e58a")});
            painter->drawText(live_pill, Qt::AlignCenter, QStringLiteral("LIVE"));
            right_occupied = 46;
        } else if (ordinal > 0) {
            const QString ord_text = QStringLiteral("%1").arg(ordinal, 2, 10, QLatin1Char('0'));
            QFont ord_font = option.font;
            ord_font.setPixelSize(10);
            ord_font.setWeight(QFont::Medium);
            painter->setFont(ord_font);
            painter->setPen(selected ? QColor{QStringLiteral("#9ec5ff")}
                                     : QColor{QStringLiteral("#5c616d")});
            const QRect ord_rect{bounds.right() - 28, bounds.top(), 20, bounds.height()};
            painter->drawText(ord_rect, Qt::AlignVCenter | Qt::AlignRight, ord_text);
            right_occupied = 32;
        }

        // Title + Metadata text region
        const int text_left = static_cast<int>(badge_rect.right()) + 9;
        const int text_width = std::max(40, bounds.right() - right_occupied - text_left);

        QFont title_font = option.font;
        title_font.setPixelSize(12);
        title_font.setWeight(selected || is_live ? QFont::DemiBold : QFont::Medium);
        painter->setFont(title_font);
        painter->setPen(selected || is_live ? QColor{QStringLiteral("#ffffff")}
                                            : QColor{QStringLiteral("#e6e8ec")});
        const QFontMetrics title_fm{title_font};
        const QString elided_title = title_fm.elidedText(title, Qt::ElideRight, text_width);
        const QRect title_rect{text_left, bounds.top() + 7, text_width, 16};
        painter->drawText(title_rect, Qt::AlignVCenter | Qt::AlignLeft, elided_title);

        QString subtitle = is_song ? QStringLiteral("Song") : QStringLiteral("Sermon");
        if (slide_count > 0) {
            subtitle += QStringLiteral(" · %1 slide%2")
                            .arg(slide_count)
                            .arg(slide_count == 1 ? QString() : QStringLiteral("s"));
        }
        if (is_song && has_bg) {
            subtitle += QStringLiteral(" · Bg");
        }
        if (drop_target) {
            subtitle = QStringLiteral("Drop to apply background");
        }

        QFont sub_font = option.font;
        sub_font.setPixelSize(10);
        painter->setFont(sub_font);
        painter->setPen(drop_target
                            ? QColor{QStringLiteral("#65d6ff")}
                            : selected ? QColor{QStringLiteral("#a7c7f5")}
                                       : QColor{QStringLiteral("#868b96")});
        const QFontMetrics sub_fm{sub_font};
        const QString elided_sub = sub_fm.elidedText(subtitle, Qt::ElideRight, text_width);
        const QRect sub_rect{text_left, bounds.top() + 24, text_width, 15};
        painter->drawText(sub_rect, Qt::AlignVCenter | Qt::AlignLeft, elided_sub);

        painter->restore();
    }
};

class ServiceOrderListWidget final : public QListWidget {
public:
    using AssetDropHandler = std::function<bool(const QMimeData*, QPoint)>;
    using ContentDropHandler = std::function<bool(const QMimeData*)>;

    explicit ServiceOrderListWidget(QWidget* parent = nullptr) : QListWidget(parent) {
        viewport()->installEventFilter(this);
        setMouseTracking(true);
        setItemDelegate(new ServiceOrderItemDelegate{this});
    }

    void set_asset_drop_handler(AssetDropHandler handler) {
        asset_drop_handler_ = std::move(handler);
    }

    void set_content_drop_handler(ContentDropHandler handler) {
        content_drop_handler_ = std::move(handler);
    }

    void set_key_action_handlers(std::function<void(int)> move_delta,
                                 std::function<void()> remove_selected,
                                 std::function<void()> edit_selected) {
        key_move_delta_ = std::move(move_delta);
        key_remove_ = std::move(remove_selected);
        key_edit_ = std::move(edit_selected);
    }

    void set_viewport_resized_handler(std::function<void()> handler) {
        viewport_resized_ = std::move(handler);
    }

protected:
    void set_drop_hover_state(const QMimeData* data, QPoint pos) {
        clear_drop_hover_state();
        if (asset_id_from_mime_data(data).has_value()) {
            if (auto* target = itemAt(pos); target != nullptr) {
                target->setData(kDropTargetRole, true);
                hovered_drop_row_ = row(target);
            }
        } else if (content_ref_from_mime_data(data).has_value()) {
            content_drop_active_ = true;
        }
        viewport()->update();
    }

    void clear_drop_hover_state() {
        if (hovered_drop_row_ >= 0 && hovered_drop_row_ < count()) {
            item(hovered_drop_row_)->setData(kDropTargetRole, false);
        }
        hovered_drop_row_ = -1;
        content_drop_active_ = false;
        viewport()->update();
    }

    bool handle_drop(const QMimeData* data, QPoint position) {
        clear_drop_hover_state();
        if (asset_id_from_mime_data(data).has_value()) {
            return asset_drop_handler_ && asset_drop_handler_(data, position);
        }
        if (content_ref_from_mime_data(data).has_value()) {
            return content_drop_handler_ && content_drop_handler_(data);
        }
        return false;
    }

    bool accepts_drop(const QMimeData* data) const {
        return asset_id_from_mime_data(data).has_value() ||
               content_ref_from_mime_data(data).has_value();
    }

    bool eventFilter(QObject* watched, QEvent* event) override {
        if (watched == viewport()) {
            if (event->type() == QEvent::Resize) {
                if (viewport_resized_) {
                    viewport_resized_();
                }
            } else if (event->type() == QEvent::Drop) {
                auto* drop_event = static_cast<QDropEvent*>(event);
                if (!handle_drop(drop_event->mimeData(), drop_event->position().toPoint())) {
                    drop_event->ignore();
                } else {
                    drop_event->acceptProposedAction();
                }
                return true;
            } else if (event->type() == QEvent::DragEnter) {
                auto* drag_event = static_cast<QDragEnterEvent*>(event);
                if (accepts_drop(drag_event->mimeData())) {
                    set_drop_hover_state(drag_event->mimeData(), drag_event->position().toPoint());
                    drag_event->acceptProposedAction();
                } else {
                    clear_drop_hover_state();
                    drag_event->ignore();
                }
                return true;
            } else if (event->type() == QEvent::DragMove) {
                auto* drag_event = static_cast<QDragMoveEvent*>(event);
                if (accepts_drop(drag_event->mimeData())) {
                    set_drop_hover_state(drag_event->mimeData(), drag_event->position().toPoint());
                    drag_event->acceptProposedAction();
                } else {
                    clear_drop_hover_state();
                    drag_event->ignore();
                }
                return true;
            } else if (event->type() == QEvent::DragLeave) {
                clear_drop_hover_state();
                return true;
            }
        }
        return QListWidget::eventFilter(watched, event);
    }

    bool viewportEvent(QEvent* event) override {
        if (event->type() == QEvent::DragEnter) {
            auto* drag_event = static_cast<QDragEnterEvent*>(event);
            if (accepts_drop(drag_event->mimeData())) {
                set_drop_hover_state(drag_event->mimeData(), drag_event->position().toPoint());
                drag_event->acceptProposedAction();
            } else {
                clear_drop_hover_state();
                drag_event->ignore();
            }
            return true;
        }
        if (event->type() == QEvent::DragMove) {
            auto* drag_event = static_cast<QDragMoveEvent*>(event);
            if (accepts_drop(drag_event->mimeData())) {
                set_drop_hover_state(drag_event->mimeData(), drag_event->position().toPoint());
                drag_event->acceptProposedAction();
            } else {
                clear_drop_hover_state();
                drag_event->ignore();
            }
            return true;
        }
        if (event->type() == QEvent::DragLeave) {
            clear_drop_hover_state();
            return true;
        }
        if (event->type() == QEvent::Drop) {
            auto* drop_event = static_cast<QDropEvent*>(event);
            if (!handle_drop(drop_event->mimeData(), drop_event->position().toPoint())) {
                drop_event->ignore();
            } else {
                drop_event->acceptProposedAction();
            }
            return true;
        }
        return QListWidget::viewportEvent(event);
    }

    void paintEvent(QPaintEvent* event) override {
        QListWidget::paintEvent(event);
        if (content_drop_active_) {
            QPainter p{viewport()};
            p.setRenderHint(QPainter::Antialiasing, true);
            p.setPen(QPen{QColor{QStringLiteral("#0a84ff")}, 2.0});
            p.drawRoundedRect(viewport()->rect().adjusted(2, 2, -2, -2), 4.0, 4.0);
        }
    }

    void dragEnterEvent(QDragEnterEvent* event) override {
        if (accepts_drop(event->mimeData())) {
            set_drop_hover_state(event->mimeData(), event->position().toPoint());
            event->acceptProposedAction();
        } else {
            clear_drop_hover_state();
            event->ignore();
        }
    }

    void dragMoveEvent(QDragMoveEvent* event) override {
        if (accepts_drop(event->mimeData())) {
            set_drop_hover_state(event->mimeData(), event->position().toPoint());
            event->acceptProposedAction();
        } else {
            clear_drop_hover_state();
            event->ignore();
        }
    }

    void dragLeaveEvent(QDragLeaveEvent* event) override {
        clear_drop_hover_state();
        event->accept();
    }

    void dropEvent(QDropEvent* event) override {
        if (!handle_drop(event->mimeData(), event->position().toPoint())) {
            event->ignore();
            return;
        }
        event->acceptProposedAction();
    }

    void keyPressEvent(QKeyEvent* event) override {
        if ((event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace) && key_remove_) {
            key_remove_();
            event->accept();
            return;
        }
        if (event->modifiers().testFlag(Qt::AltModifier) && event->key() == Qt::Key_Up && key_move_delta_) {
            key_move_delta_(-1);
            event->accept();
            return;
        }
        if (event->modifiers().testFlag(Qt::AltModifier) && event->key() == Qt::Key_Down && key_move_delta_) {
            key_move_delta_(1);
            event->accept();
            return;
        }
        if ((event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) &&
            event->modifiers().testFlag(Qt::ControlModifier) && key_edit_) {
            key_edit_();
            event->accept();
            return;
        }
        QListWidget::keyPressEvent(event);
    }

private:
    AssetDropHandler asset_drop_handler_;
    ContentDropHandler content_drop_handler_;
    std::function<void(int)> key_move_delta_;
    std::function<void()> key_remove_;
    std::function<void()> key_edit_;
    std::function<void()> viewport_resized_;
    int hovered_drop_row_{-1};
    bool content_drop_active_{false};
};

}  // namespace

ServiceOrderWidget::ServiceOrderWidget(QWidget* parent) : QWidget(parent) {
    setObjectName(QStringLiteral("service-order"));
    setStyleSheet(QStringLiteral(
        "#service-order {"
        "  background-color: #1b1c21;"
        "  border-right: 1px solid #111215;"
        "  color: #e6e8ec;"
        "}"
        "#service-order QListWidget#service-order-list {"
        "  background-color: #1b1c21;"
        "  border: none;"
        "  outline: 0;"
        "}"
        "#service-order QPushButton[orderTool=\"true\"] {"
        "  background-color: #272930;"
        "  border: 1px solid #353842;"
        "  border-radius: 4px;"
        "  color: #dfe2e7;"
        "  font-size: 11px;"
        "  font-weight: 600;"
        "  padding: 2px 6px;"
        "}"
        "#service-order QPushButton[orderTool=\"true\"]:hover {"
        "  background-color: #32353f;"
        "  border-color: #4b4f5c;"
        "}"
        "#service-order QPushButton[orderTool=\"true\"]:pressed {"
        "  background-color: #1d1f25;"
        "}"
        "#service-order QPushButton[orderTool=\"true\"]:disabled {"
        "  background-color: #1f2026;"
        "  border-color: #272930;"
        "  color: #555965;"
        "}"
        "#service-order QPushButton#service-order-remove:hover:enabled {"
        "  background-color: #5b2226;"
        "  border-color: #9f3338;"
        "  color: #ffffff;"
        "}"
        "#service-order QLineEdit#service-order-filter {"
        "  background-color: #15161a;"
        "  border: 1px solid #2c2f38;"
        "  border-radius: 4px;"
        "  color: #e8eaed;"
        "  font-size: 11px;"
        "  padding: 3px 8px;"
        "}"
        "#service-order QLineEdit#service-order-filter:focus {"
        "  border-color: #0a84ff;"
        "}"));

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // 1. Header Bar
    auto* header_bar = new QWidget{this};
    header_bar->setObjectName(QStringLiteral("service-order-header"));
    header_bar->setFixedHeight(26);
    header_bar->setStyleSheet(QStringLiteral(
        "#service-order-header { background-color: #191a1e; border-bottom: 1px solid #111215; }"));
    auto* header_layout = new QHBoxLayout(header_bar);
    header_layout->setContentsMargins(10, 0, 6, 0);
    header_layout->setSpacing(6);

    auto* title_label = new QLabel{QStringLiteral("SERVICE ORDER"), header_bar};
    title_label->setObjectName(QStringLiteral("service-order-title"));
    title_label->setStyleSheet(QStringLiteral(
        "color: #8e929b; font-size: 10px; font-weight: 700; letter-spacing: 1.1px; border: none;"));
    header_layout->addWidget(title_label);
    header_layout->addStretch(1);

    count_badge_ = new QLabel{QStringLiteral("0 ITEMS"), header_bar};
    count_badge_->setObjectName(QStringLiteral("service-order-count"));
    count_badge_->setFixedHeight(18);
    count_badge_->setAlignment(Qt::AlignCenter);
    count_badge_->setStyleSheet(QStringLiteral(
        "background-color: #25272e; color: #9ea3ae; border: 1px solid #30333d; "
        "border-radius: 3px; padding: 0px 6px; font-size: 9px; font-weight: 700;"));
    header_layout->addWidget(count_badge_, 0, Qt::AlignVCenter);

    add_button_ = new QPushButton{header_bar};
    add_button_->setObjectName(QStringLiteral("service-order-add"));
    add_button_->setFixedSize(20, 20);
    add_button_->setCursor(Qt::PointingHandCursor);
    add_button_->setToolTip(QStringLiteral("New or Import Presentation Item"));
    add_button_->setIcon(make_order_icon(OrderGlyph::plus, QColor{QStringLiteral("#c5c9d2")}, 13));
    add_button_->setIconSize(QSize{13, 13});
    add_button_->setStyleSheet(QStringLiteral(
        "QPushButton { background: transparent; border: none; border-radius: 3px; }"
        "QPushButton:hover { background-color: #2d3039; }"
        "QPushButton:disabled { opacity: 0.4; }"));
    add_button_->setEnabled(false);
    QObject::connect(add_button_, &QPushButton::clicked, this, [this] { open_add_menu(); });
    header_layout->addWidget(add_button_);
    layout->addWidget(header_bar);

    // 2. Service Order List + Transparent Empty-State Overlay
    auto* list_widget = new ServiceOrderListWidget{this};
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
    empty_overlay_->setStyleSheet(QStringLiteral("background: transparent;"));
    auto* empty_layout = new QVBoxLayout(empty_overlay_);
    empty_layout->setContentsMargins(18, 24, 18, 24);
    empty_layout->setSpacing(6);
    empty_layout->addStretch(1);

    auto* empty_icon = new QLabel{empty_overlay_};
    empty_icon->setAlignment(Qt::AlignCenter);
    empty_icon->setPixmap(
        make_order_icon(OrderGlyph::empty_stack, QColor{QStringLiteral("#4f5460")}, 28).pixmap(28, 28));
    empty_layout->addWidget(empty_icon);

    empty_title_label_ = new QLabel{QStringLiteral("Service Order Empty"), empty_overlay_};
    empty_title_label_->setAlignment(Qt::AlignCenter);
    empty_title_label_->setStyleSheet(
        QStringLiteral("color: #c5c9d2; font-size: 12px; font-weight: 600;"));
    empty_layout->addWidget(empty_title_label_);

    empty_hint_label_ = new QLabel{
        QStringLiteral("Drag a Song or Sermon here, or double-click an item in the Library below."),
        empty_overlay_};
    empty_hint_label_->setAlignment(Qt::AlignCenter);
    empty_hint_label_->setWordWrap(true);
    empty_hint_label_->setStyleSheet(QStringLiteral("color: #777c87; font-size: 11px;"));
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
    controls_bar->setStyleSheet(QStringLiteral(
        "#service-order-controls {"
        "  background-color: #202228;"
        "  border-top: 1px solid #131417;"
        "}"));
    auto* controls = new QHBoxLayout(controls_bar);
    controls->setContentsMargins(6, 4, 6, 4);
    controls->setSpacing(4);

    move_up_button_ = new QPushButton{controls_bar};
    move_up_button_->setObjectName(QStringLiteral("service-order-move-up"));
    move_up_button_->setProperty("orderTool", true);
    move_up_button_->setFixedSize(28, 24);
    move_up_button_->setCursor(Qt::PointingHandCursor);
    move_up_button_->setToolTip(QStringLiteral("Move Item Up (Alt+Up)"));
    move_up_button_->setIcon(
        make_order_icon(OrderGlyph::chevron_up, QColor{QStringLiteral("#dfe2e7")}, 13));
    move_up_button_->setIconSize(QSize{13, 13});

    move_down_button_ = new QPushButton{controls_bar};
    move_down_button_->setObjectName(QStringLiteral("service-order-move-down"));
    move_down_button_->setProperty("orderTool", true);
    move_down_button_->setFixedSize(28, 24);
    move_down_button_->setCursor(Qt::PointingHandCursor);
    move_down_button_->setToolTip(QStringLiteral("Move Item Down (Alt+Down)"));
    move_down_button_->setIcon(
        make_order_icon(OrderGlyph::chevron_down, QColor{QStringLiteral("#dfe2e7")}, 13));
    move_down_button_->setIconSize(QSize{13, 13});

    edit_button_ = new QPushButton{controls_bar};
    edit_button_->setObjectName(QStringLiteral("service-order-edit"));
    edit_button_->setProperty("orderTool", true);
    edit_button_->setFixedSize(28, 24);
    edit_button_->setCursor(Qt::PointingHandCursor);
    edit_button_->setToolTip(QStringLiteral("Edit Selected Song or Sermon (Ctrl+Enter)"));
    edit_button_->setIcon(
        make_order_icon(OrderGlyph::edit, QColor{QStringLiteral("#dfe2e7")}, 13));
    edit_button_->setIconSize(QSize{13, 13});

    remove_button_ = new QPushButton{QStringLiteral("Remove"), controls_bar};
    remove_button_->setObjectName(QStringLiteral("service-order-remove"));
    remove_button_->setProperty("orderTool", true);
    remove_button_->setFixedHeight(24);
    remove_button_->setCursor(Qt::PointingHandCursor);
    remove_button_->setToolTip(QStringLiteral("Remove Selected Item from Service Order (Del)"));
    remove_button_->setIcon(
        make_order_icon(OrderGlyph::trash, QColor{QStringLiteral("#d98b8e")}, 12));
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
    filter_bar->setStyleSheet(QStringLiteral(
        "#service-order-filter-bar {"
        "  background-color: #23252a;"
        "  border-top: 1px solid #131417;"
        "}"));
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

    update_empty_state();
    update_controls_state();
}

void ServiceOrderWidget::set_items(std::vector<ServiceOrderView> items) {
    items_ = std::move(items);
    list_->clear();
    for (std::size_t index = 0; index != items_.size(); ++index) {
        auto& view = items_[index];
        if (live_content_.has_value()) {
            view.is_live = (view.item.content == *live_content_);
        }
        auto* item = new QListWidgetItem{view.label};
        item->setData(kKindRole, static_cast<int>(view.item.content.kind));
        item->setData(kSlideCountRole, view.slide_count);
        item->setData(kHasBgRole, view.has_background);
        item->setData(kIsLiveRole, view.is_live);
        item->setData(kOrdinalRole, static_cast<int>(index + 1));
        item->setData(kDropTargetRole, false);
        const QString kind_name = view.item.content.kind == domain::ContentKind::song
                                      ? QStringLiteral("Song")
                                      : QStringLiteral("Sermon");
        QString tip = QStringLiteral("%1 (%2)").arg(view.label, kind_name);
        if (view.slide_count > 0) {
            tip += QStringLiteral("\n%1 slide%2")
                       .arg(view.slide_count)
                       .arg(view.slide_count == 1 ? QString() : QStringLiteral("s"));
        }
        item->setToolTip(tip);
        list_->addItem(item);
    }
    count_badge_->setText(
        QStringLiteral("%1 ITEM%2")
            .arg(static_cast<qulonglong>(items_.size()))
            .arg(items_.size() == 1 ? QString() : QStringLiteral("S")));
    apply_filter();
    update_controls_state();
}

void ServiceOrderWidget::set_live_content(std::optional<domain::ContentRef> live_content) {
    live_content_ = std::move(live_content);
    for (int i = 0; i < list_->count() && i < static_cast<int>(items_.size()); ++i) {
        const bool live = live_content_.has_value() &&
                          items_[static_cast<std::size_t>(i)].item.content == *live_content_;
        items_[static_cast<std::size_t>(i)].is_live = live;
        list_->item(i)->setData(kIsLiveRole, live);
    }
    list_->viewport()->update();
}

void ServiceOrderWidget::set_authoring_handlers(std::function<void()> new_song,
                                                std::function<void()> import_song,
                                                std::function<void()> new_sermon,
                                                std::function<void()> import_sermon) {
    new_song_handler_ = std::move(new_song);
    import_song_handler_ = std::move(import_song);
    new_sermon_handler_ = std::move(new_sermon);
    import_sermon_handler_ = std::move(import_sermon);
    const bool any = static_cast<bool>(new_song_handler_) ||
                     static_cast<bool>(import_song_handler_) ||
                     static_cast<bool>(new_sermon_handler_) ||
                     static_cast<bool>(import_sermon_handler_);
    add_button_->setEnabled(any);
}

void ServiceOrderWidget::select_row(int row) {
    list_->setCurrentRow(row);
    update_controls_state();
}

int ServiceOrderWidget::count() const noexcept { return list_->count(); }

void ServiceOrderWidget::apply_filter() {
    const QString query = filter_input_->text().trimmed();
    for (int i = 0; i < list_->count() && i < static_cast<int>(items_.size()); ++i) {
        auto* item = list_->item(i);
        const auto kind_label = items_[static_cast<std::size_t>(i)].item.content.kind ==
                                        domain::ContentKind::song
                                    ? QStringLiteral("song")
                                    : QStringLiteral("sermon");
        const bool matches = query.isEmpty() ||
                             item->text().contains(query, Qt::CaseInsensitive) ||
                             kind_label.contains(query, Qt::CaseInsensitive);
        item->setHidden(!matches);
    }
    update_empty_state();
    update_controls_state();
}

void ServiceOrderWidget::update_empty_state() {
    if (empty_overlay_ == nullptr) {
        return;
    }
    empty_overlay_->setGeometry(list_->viewport()->rect());
    if (items_.empty()) {
        empty_title_label_->setText(QStringLiteral("Service Order Empty"));
        empty_hint_label_->setText(QStringLiteral(
            "Drag a Song or Sermon here, or double-click an item in the Library below."));
        empty_overlay_->setVisible(true);
        return;
    }
    bool any_visible = false;
    for (int i = 0; i < list_->count(); ++i) {
        if (!list_->item(i)->isHidden()) {
            any_visible = true;
            break;
        }
    }
    if (!any_visible) {
        empty_title_label_->setText(QStringLiteral("No Matching Items"));
        empty_hint_label_->setText(QStringLiteral("Clear the filter to see all service items."));
        empty_overlay_->setVisible(true);
    } else {
        empty_overlay_->setVisible(false);
    }
}

void ServiceOrderWidget::update_controls_state() {
    const int row = list_->currentRow();
    const bool valid = row >= 0 && row < static_cast<int>(items_.size());
    const bool filtering = !filter_input_->text().trimmed().isEmpty();
    move_up_button_->setEnabled(valid && !filtering && row > 0);
    move_down_button_->setEnabled(valid && !filtering && row + 1 < static_cast<int>(items_.size()));
    edit_button_->setEnabled(valid && static_cast<bool>(edit_handler_));
    remove_button_->setEnabled(valid);
}

void ServiceOrderWidget::move_selected(int delta) {
    const int row = list_->currentRow();
    const int target = row + delta;
    if (row >= 0 && row < static_cast<int>(items_.size()) &&
        target >= 0 && target < static_cast<int>(items_.size()) &&
        move_handler_) {
        move_handler_(items_[static_cast<std::size_t>(row)].item.id, target);
    }
}

void ServiceOrderWidget::remove_selected() {
    const int row = list_->currentRow();
    if (row >= 0 && row < static_cast<int>(items_.size()) && remove_handler_) {
        remove_handler_(items_[static_cast<std::size_t>(row)].item.id);
    }
}

void ServiceOrderWidget::edit_selected() {
    const int row = list_->currentRow();
    if (row >= 0 && row < static_cast<int>(items_.size()) && edit_handler_) {
        edit_handler_(items_[static_cast<std::size_t>(row)].item.content);
    }
}

void ServiceOrderWidget::open_add_menu() {
    QMenu menu{this};
    if (new_song_handler_) {
        auto* action = menu.addAction(QStringLiteral("New Song..."));
        QObject::connect(action, &QAction::triggered, this, [this] { new_song_handler_(); });
    }
    if (import_song_handler_) {
        auto* action = menu.addAction(QStringLiteral("Import Song Lyrics..."));
        QObject::connect(action, &QAction::triggered, this, [this] { import_song_handler_(); });
    }
    if ((new_song_handler_ || import_song_handler_) &&
        (new_sermon_handler_ || import_sermon_handler_)) {
        menu.addSeparator();
    }
    if (new_sermon_handler_) {
        auto* action = menu.addAction(QStringLiteral("New Sermon..."));
        QObject::connect(action, &QAction::triggered, this, [this] { new_sermon_handler_(); });
    }
    if (import_sermon_handler_) {
        auto* action = menu.addAction(QStringLiteral("Import Sermon Deck..."));
        QObject::connect(action, &QAction::triggered, this, [this] { import_sermon_handler_(); });
    }
    if (menu.isEmpty()) {
        return;
    }
    menu.exec(add_button_->mapToGlobal(QPoint{0, add_button_->height()}));
}

void ServiceOrderWidget::open_context_menu(const QPoint& viewport_pos) {
    auto* clicked = list_->itemAt(viewport_pos);
    if (clicked == nullptr) {
        return;
    }
    const int row = list_->row(clicked);
    if (row < 0 || row >= static_cast<int>(items_.size())) {
        return;
    }
    list_->setCurrentRow(row);

    QMenu menu{this};
    const bool filtering = !filter_input_->text().trimmed().isEmpty();
    auto* up_action = menu.addAction(QStringLiteral("Move Up"));
    up_action->setEnabled(!filtering && row > 0);
    QObject::connect(up_action, &QAction::triggered, this, [this] { move_selected(-1); });

    auto* down_action = menu.addAction(QStringLiteral("Move Down"));
    down_action->setEnabled(!filtering && row + 1 < static_cast<int>(items_.size()));
    QObject::connect(down_action, &QAction::triggered, this, [this] { move_selected(1); });

    if (edit_handler_) {
        menu.addSeparator();
        const bool is_song =
            items_[static_cast<std::size_t>(row)].item.content.kind == domain::ContentKind::song;
        auto* edit_action = menu.addAction(is_song ? QStringLiteral("Edit Song...")
                                                   : QStringLiteral("Edit Sermon..."));
        QObject::connect(edit_action, &QAction::triggered, this, [this] { edit_selected(); });
    }

    menu.addSeparator();
    auto* remove_action = menu.addAction(QStringLiteral("Remove from Service Order"));
    QObject::connect(remove_action, &QAction::triggered, this, [this] { remove_selected(); });

    menu.exec(list_->viewport()->mapToGlobal(viewport_pos));
}

}  // namespace shekinah::ui
